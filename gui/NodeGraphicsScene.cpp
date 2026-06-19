// gui/NodeGraphicsScene.cpp
#include "NodeGraphicsScene.h"
#include "NodeGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "../core/WorkflowGraph.h"
#include "../core/NodeBase.h"
#include "../core/NodeFactory.h"
#include "../common/Logger.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <algorithm>

NodeGraphicsScene::NodeGraphicsScene(WorkflowGraph* graph, QObject* parent)
    : QGraphicsScene(parent)
    , m_graph(graph)
{
    setSceneRect(-5000, -5000, 10000, 10000);

    // 连线更新节流定时器
    m_connectionUpdateTimer = new QTimer(this);
    m_connectionUpdateTimer->setSingleShot(true);
    m_connectionUpdateTimer->setInterval(50); // 降低刷新率减少拖影
    connect(m_connectionUpdateTimer, &QTimer::timeout, this, [this]() {
        if (m_connectionsDirty) {
            updateAllConnections();
            m_connectionsDirty = false;
        }
    });

    // 监听图模型变化
    connect(m_graph, &WorkflowGraph::nodeAdded,
            this, &NodeGraphicsScene::onGraphNodeAdded);
    connect(m_graph, &WorkflowGraph::nodeRemoved,
            this, &NodeGraphicsScene::onGraphNodeRemoved);
    connect(m_graph, &WorkflowGraph::connectionAdded,
            this, &NodeGraphicsScene::onGraphConnectionAdded);
    connect(m_graph, &WorkflowGraph::connectionRemoved,
            this, &NodeGraphicsScene::onGraphConnectionRemoved);
    connect(m_graph, &WorkflowGraph::nodePortsChanged,
            this, [this](const QString& nodeId) {
        if (auto* ni = nodeItemById(nodeId)) ni->updateLayout();
    });
}

NodeGraphicsScene::~NodeGraphicsScene()
{
    // Qt自动断开信号，无需手动disconnect（避免m_graph已析构时的dangling pointer）
    // 清理所有图形项
    for (auto* conn : m_connectionItems) {
        removeItem(conn);
        delete conn;
    }
    m_connectionItems.clear();

    for (auto it = m_nodeItems.begin(); it != m_nodeItems.end(); ++it) {
        removeItem(it.value());
        delete it.value();
    }
    m_nodeItems.clear();
}

NodeGraphicsItem* NodeGraphicsScene::addNodeItem(NodeBase* node)
{
    auto* item = new NodeGraphicsItem(node);
    addItem(item);
    m_nodeItems[node->id()] = item;
    return item;
}

void NodeGraphicsScene::removeNodeItem(const QString& nodeId)
{
    auto it = m_nodeItems.find(nodeId);
    if (it != m_nodeItems.end()) {
        NodeGraphicsItem* item = it.value();
        m_nodeItems.erase(it);
        removeItem(item);
        delete item;
    }
}

NodeGraphicsItem* NodeGraphicsScene::nodeItemById(const QString& nodeId) const
{
    return m_nodeItems.value(nodeId, nullptr);
}

ConnectionGraphicsItem* NodeGraphicsScene::addConnectionItem(const Connection& conn)
{
    auto* srcNode = nodeItemById(conn.sourceNodeId());
    auto* dstNode = nodeItemById(conn.targetNodeId());
    if (!srcNode || !dstNode) return nullptr;

    auto* srcPort = srcNode->outputPortItem(conn.sourcePortName());
    auto* dstPort = dstNode->inputPortItem(conn.targetPortName());
    if (!srcPort || !dstPort) return nullptr;

    auto* item = new ConnectionGraphicsItem(srcPort, dstPort);
    addItem(item);
    m_connectionItems.append(item);
    return item;
}

void NodeGraphicsScene::removeConnectionItem(const Connection& conn)
{
    for (int i = m_connectionItems.size() - 1; i >= 0; --i) {
        auto* item = m_connectionItems[i];
        if (item->toConnection() == conn) {
            m_connectionItems.removeAt(i);
            removeItem(item);
            delete item;
            return;
        }
    }
}

void NodeGraphicsScene::removeConnectionItem(ConnectionGraphicsItem* item)
{
    if (!item) return;
    m_connectionItems.removeOne(item);
    removeItem(item);
    delete item;
}

void NodeGraphicsScene::requestConnectionUpdate()
{
    m_connectionsDirty = true;
    if (!m_connectionUpdateTimer->isActive())
        m_connectionUpdateTimer->start();
}

void NodeGraphicsScene::updateAllConnections()
{
    for (auto* item : m_connectionItems) {
        item->updatePath();
    }
}

QVector<ConnectionGraphicsItem*> NodeGraphicsScene::allConnectionItems() const
{
    return m_connectionItems;
}

void NodeGraphicsScene::startConnectionDrag(PortGraphicsItem* sourcePort)
{
    m_connectionDragging = true;
    m_dragSourcePort = sourcePort;

    m_tempConnection = new QGraphicsPathItem();
    m_tempConnection->setZValue(-1);
    QPen pen(sourcePort->portColor(), 2, Qt::DashLine);
    pen.setColor(QColor(sourcePort->portColor().red(),
                        sourcePort->portColor().green(),
                        sourcePort->portColor().blue(), 150));
    m_tempConnection->setPen(pen);
    addItem(m_tempConnection);

    emit connectionDragStarted();
}

void NodeGraphicsScene::updateConnectionDrag(const QPointF& scenePos)
{
    if (!m_connectionDragging || !m_tempConnection || !m_dragSourcePort)
        return;

    QPointF p0 = m_dragSourcePort->centerInScene();

    qreal dx = qAbs(scenePos.x() - p0.x());
    qreal offset = qBound(50.0, dx * 0.4, 150.0);

    QPainterPath path;
    path.moveTo(p0);
    path.cubicTo(QPointF(p0.x() + offset, p0.y()),
                 QPointF(scenePos.x() - offset, scenePos.y()),
                 scenePos);
    m_tempConnection->setPath(path);

    // 高亮/拒绝目标端口
    PortGraphicsItem* targetPort = findPortAt(scenePos);
    // 重置所有端口
    for (auto* nodeItem : m_nodeItems) {
        for (auto* p : nodeItem->allInputPortItems()) {
            p->resetAppearance();
        }
        for (auto* p : nodeItem->allOutputPortItems()) {
            p->resetAppearance();
        }
    }
    if (targetPort && !targetPort->isOutput()) {
        QString errorMsg;
        if (checkConnectionCompatible(m_dragSourcePort, targetPort, errorMsg)) {
            targetPort->setHighlighted(true);
        } else {
            targetPort->setRejected(true);
            emit statusMessage(errorMsg);
        }
    }
}

void NodeGraphicsScene::finishConnectionDrag(PortGraphicsItem* targetPort)
{
    if (!m_connectionDragging || !m_dragSourcePort) {
        cancelConnectionDrag();
        return;
    }

    if (targetPort && !targetPort->isOutput()) {
        QString errorMsg;
        if (checkConnectionCompatible(m_dragSourcePort, targetPort, errorMsg)) {
            m_graph->addConnection(
                m_dragSourcePort->parentNodeItem()->nodeId(),
                m_dragSourcePort->portDef().name,
                targetPort->parentNodeItem()->nodeId(),
                targetPort->portDef().name
            );
            emit connectionDragFinished(true);
        } else {
            Logger::instance().warning("连线被拒绝: " + errorMsg);
            emit statusMessage("连线被拒绝: " + errorMsg);
            emit connectionDragFinished(false);
        }
    } else if (m_dragSourcePort->isOutput()) {
        // 从输出端口拖到空白处：断开该端口所有连线
        QString id = m_dragSourcePort->parentNodeItem()->nodeId();
        QString port = m_dragSourcePort->portDef().name;
        auto conns = m_graph->findConnectionsFromOutput(id, port);
        for (const auto& c : conns) {
            m_graph->removeConnection(c.sourceNodeId(), c.sourcePortName(),
                                       c.targetNodeId(), c.targetPortName());
        }
        emit connectionDragFinished(true);
    } else {
        // 从输入端口拖到空白处：断开连入该端口的连线
        QString id = m_dragSourcePort->parentNodeItem()->nodeId();
        QString port = m_dragSourcePort->portDef().name;
        Connection* conn = m_graph->findConnectionToInput(id, port);
        if (conn) {
            m_graph->removeConnection(conn->sourceNodeId(), conn->sourcePortName(),
                                       conn->targetNodeId(), conn->targetPortName());
        }
        emit connectionDragFinished(true);
    }

    cancelConnectionDrag();
}

void NodeGraphicsScene::cancelConnectionDrag()
{
    m_connectionDragging = false;
    m_dragSourcePort = nullptr;

    if (m_tempConnection) {
        removeItem(m_tempConnection);
        delete m_tempConnection;
        m_tempConnection = nullptr;
    }

    // 重置所有端口外观
    for (auto* nodeItem : m_nodeItems) {
        for (auto* p : nodeItem->allInputPortItems())
            p->resetAppearance();
        for (auto* p : nodeItem->allOutputPortItems())
            p->resetAppearance();
    }
}

PortGraphicsItem* NodeGraphicsScene::findPortAt(const QPointF& scenePos) const
{
    // 遍历所有节点找端口
    for (auto* nodeItem : m_nodeItems) {
        // 检查输入端口
        for (auto* p : nodeItem->allInputPortItems()) {
            QPointF portCenter = p->centerInScene();
            if (QLineF(scenePos, portCenter).length() < 15.0)
                return p;
        }
        // 检查输出端口
        for (auto* p : nodeItem->allOutputPortItems()) {
            QPointF portCenter = p->centerInScene();
            if (QLineF(scenePos, portCenter).length() < 15.0)
                return p;
        }
    }
    return nullptr;
}

bool NodeGraphicsScene::checkConnectionCompatible(PortGraphicsItem* src, PortGraphicsItem* dst,
                                                    QString& errorMsg) const
{
    if (!src || !dst) {
        errorMsg = "无效的端口";
        return false;
    }

    // 必须从输出连到输入
    if (!src->isOutput() || dst->isOutput()) {
        errorMsg = "连线只能从输出端口连向输入端口";
        return false;
    }

    // 不能连接同一个节点
    if (src->parentNodeItem() == dst->parentNodeItem()) {
        errorMsg = "不能连接同一个节点的端口";
        return false;
    }

    // 检查类型兼容
    DataType srcType = src->portDef().dataType;
    if (!dst->portDef().accepts(srcType)) {
        errorMsg = QString("无法连接：输出类型为%1，但目标端口接受%2")
            .arg(dataTypeName(srcType))
            .arg([&]() {
                QStringList types;
                for (auto t : dst->portDef().acceptedTypes)
                    types << dataTypeName(t);
                return types.join(", ");
            }());
        return false;
    }

    return true;
}

void NodeGraphicsScene::onNodeMoved(NodeGraphicsItem* item)
{
    Q_UNUSED(item)
    updateAllConnections();
}

void NodeGraphicsScene::onNodeSelected(NodeGraphicsItem* item)
{
    emit nodeSelected(item->nodeId());

    // 连线高亮/暗淡效果
    QSet<QString> relatedNodeIds;
    for (auto* conn : m_connectionItems) {
        auto c = conn->toConnection();
        if (c.sourceNodeId() == item->nodeId() || c.targetNodeId() == item->nodeId()) {
            relatedNodeIds.insert(c.sourceNodeId());
            relatedNodeIds.insert(c.targetNodeId());
        }
    }

    for (auto* conn : m_connectionItems) {
        auto c = conn->toConnection();
        bool related = relatedNodeIds.contains(c.sourceNodeId())
                    && relatedNodeIds.contains(c.targetNodeId());
        conn->setDimmed(!related);
    }
}

void NodeGraphicsScene::onNodeDoubleClicked(NodeGraphicsItem* item)
{
    emit nodeDoubleClicked(item->nodeId());
}

void NodeGraphicsScene::onPreviewRequested(NodeGraphicsItem* item)
{
    emit previewRequested(item->nodeId());
}

void NodeGraphicsScene::onSettingsRequested(NodeGraphicsItem* item)
{
    emit settingsRequested(item->nodeId());
}

void NodeGraphicsScene::onGraphNodeAdded(const QString& nodeId)
{
    NodeBase* node = m_graph->nodeById(nodeId);
    if (node) {
        auto* item = addNodeItem(node);
        emit nodeAdded(nodeId);
    }
}

void NodeGraphicsScene::onGraphNodeRemoved(const QString& nodeId)
{
    auto it = m_nodeItems.find(nodeId);
    if (it != m_nodeItems.end()) {
        NodeGraphicsItem* nodeItem = it.value();
        m_nodeItems.erase(it);
        removeItem(nodeItem);
        delete nodeItem;
    }
    emit nodeRemoved(nodeId);
}

void NodeGraphicsScene::onGraphConnectionAdded(const Connection& conn)
{
    addConnectionItem(conn);
    // 刷新两端节点端口布局（支持动态端口扩展）
    if (auto* src = nodeItemById(conn.sourceNodeId())) src->updateLayout();
    if (auto* dst = nodeItemById(conn.targetNodeId())) dst->updateLayout();
}

void NodeGraphicsScene::onGraphConnectionRemoved(const Connection& conn)
{
    removeConnectionItem(conn);
    if (auto* src = nodeItemById(conn.sourceNodeId())) src->updateLayout();
    if (auto* dst = nodeItemById(conn.targetNodeId())) dst->updateLayout();
}

void NodeGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, m_bgColor);

    // 网格点
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_gridColor);

    qreal left = qFloor(rect.left() / m_gridSize) * m_gridSize;
    qreal top = qFloor(rect.top() / m_gridSize) * m_gridSize;

    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
            painter->drawEllipse(QPointF(x, y), 0.5, 0.5);
        }
    }
}

void NodeGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem* clickedItem = itemAt(event->scenePos(), QTransform());
        if (auto* portItem = dynamic_cast<PortGraphicsItem*>(clickedItem)) {
            startConnectionDrag(portItem);
            event->accept();
            return;
        }
        // 点击空白区域：开始自定义框选
        if (!clickedItem) {
            clearSelection();
            emit selectionCleared();
            m_rubberBanding = true;
            m_rubberBandOrigin = event->scenePos();
            m_rubberBandRect = new QGraphicsRectItem();
            m_rubberBandRect->setPen(QPen(QColor("#4EC9B0"), 1, Qt::DashLine));
            m_rubberBandRect->setBrush(QColor(78, 201, 176, 30));
            m_rubberBandRect->setZValue(100);
            addItem(m_rubberBandRect);
            event->accept();
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void NodeGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_connectionDragging) {
        updateConnectionDrag(event->scenePos());
        event->accept();
        return;
    }
    if (m_rubberBanding && m_rubberBandRect) {
        QRectF r = QRectF(m_rubberBandOrigin, event->scenePos()).normalized();
        m_rubberBandRect->setRect(r);
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void NodeGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_connectionDragging) {
        PortGraphicsItem* targetPort = findPortAt(event->scenePos());
        finishConnectionDrag(targetPort);
        event->accept();
        return;
    }
    if (m_rubberBanding && m_rubberBandRect) {
        QRectF selRect = m_rubberBandRect->rect();
        removeItem(m_rubberBandRect);
        delete m_rubberBandRect;
        m_rubberBandRect = nullptr;
        m_rubberBanding = false;
        // 选中框内节点
        if (selRect.width() > 5 || selRect.height() > 5) {
            QList<QGraphicsItem*> found = items(selRect, Qt::IntersectsItemBoundingRect);
            for (auto* it : found) {
                if (dynamic_cast<NodeGraphicsItem*>(it))
                    it->setSelected(true);
            }
        }
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void NodeGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // 防止默认的上下文菜单导致崩溃
    event->accept();

    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());

    if (auto* connItem = dynamic_cast<ConnectionGraphicsItem*>(item)) {
        emit connectionContextMenuRequested(connItem, event->scenePos());
        return;
    }

    if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
        emit nodeContextMenuRequested(nodeItem->nodeId(), event->scenePos());
        return;
    }

    // 空白区域
    emit canvasContextMenuRequested(event->scenePos());
}

void NodeGraphicsScene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // 先收集所有待删除的节点ID和连线，避免在迭代中删除导致崩溃
        QStringList nodeIds;
        QList<Connection> conns;

        auto selected = selectedItems();
        for (auto* item : selected) {
            if (auto* connItem = dynamic_cast<ConnectionGraphicsItem*>(item)) {
                conns.append(connItem->toConnection());
            } else if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
                nodeIds.append(nodeItem->nodeId());
            }
        }

        // 通知UI清理引用
        clearSelection();
        emit selectionCleared();

        // 先删除连线，再删除节点（检查节点是否仍存在）
        for (const auto& conn : conns) {
            m_graph->removeConnection(
                conn.sourceNodeId(), conn.sourcePortName(),
                conn.targetNodeId(), conn.targetPortName());
        }
        for (const auto& nodeId : nodeIds) {
            if (m_graph->nodeById(nodeId)) {
                m_graph->removeNode(nodeId);
            }
        }

        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        if (m_connectionDragging) cancelConnectionDrag();
        if (m_rubberBanding && m_rubberBandRect) {
            removeItem(m_rubberBandRect); delete m_rubberBandRect;
            m_rubberBandRect = nullptr; m_rubberBanding = false;
        }
        clearSelection();
        emit selectionCleared();
        event->accept();
        return;
    }

    QGraphicsScene::keyPressEvent(event);
}

// -------------------------------------------------------------------
// 拖放事件：支持从工具箱拖拽节点类型到画布
// -------------------------------------------------------------------

void NodeGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-sciflow-nodetype")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void NodeGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-sciflow-nodetype")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void NodeGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (!event->mimeData()->hasFormat("application/x-sciflow-nodetype")) {
        event->ignore();
        return;
    }

    QString typeName = QString::fromUtf8(event->mimeData()->data("application/x-sciflow-nodetype"));
    if (typeName.isEmpty()) {
        event->ignore();
        return;
    }

    m_graph->addNode(typeName, event->scenePos());
    event->acceptProposedAction();
}
