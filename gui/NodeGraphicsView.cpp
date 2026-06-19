// gui/NodeGraphicsView.cpp
#include "NodeGraphicsView.h"
#include "NodeGraphicsScene.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "../core/WorkflowGraph.h"
#include "../core/NodeBase.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QQueue>
#include <QMap>
#include <cmath>

NodeGraphicsView::NodeGraphicsView(NodeGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , m_nodeScene(scene)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);  // 不用内置RubberBand避免原生窗口闪烁
    setCacheMode(QGraphicsView::CacheBackground);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QColor("#1e1e1e"));
    setAcceptDrops(true);

    // 设置初始视口中心
    centerOn(0, 0);
}

void NodeGraphicsView::zoomIn()
{
    zoomTo(m_zoomFactor * 1.1);
}

void NodeGraphicsView::zoomOut()
{
    zoomTo(m_zoomFactor * 0.9);
}

void NodeGraphicsView::zoomToFit()
{
    auto items = m_nodeScene->nodeItemById("");
    // 计算所有节点的包围矩形
    QRectF bounds;
    bool first = true;
    for (auto* item : m_nodeScene->items()) {
        if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
            QRectF r = nodeItem->sceneBoundingRect();
            if (first) {
                bounds = r;
                first = false;
            } else {
                bounds = bounds.united(r);
            }
        }
    }
    if (!first) {
        bounds.adjust(-100, -100, 100, 100);
        fitInView(bounds, Qt::KeepAspectRatio);
        // 更新缩放因子
        m_zoomFactor = transform().m11();
        emit zoomChanged(m_zoomFactor);
    }
}

void NodeGraphicsView::zoomTo(qreal factor)
{
    factor = qBound(MIN_ZOOM, factor, MAX_ZOOM);
    qreal s = factor / m_zoomFactor;
    this->scale(s, s);
    m_zoomFactor = factor;
    emit zoomChanged(m_zoomFactor);
}

void NodeGraphicsView::autoLayoutNodes()
{
    // 收集所有节点
    QList<NodeGraphicsItem*> nodeItems;
    for (auto* item : m_nodeScene->items()) {
        if (auto* ni = dynamic_cast<NodeGraphicsItem*>(item)) {
            nodeItems.append(ni);
        }
    }
    if (nodeItems.isEmpty()) return;

    // 收集连线信息
    QMap<QString, QStringList> successors; // nodeId → 后继节点列表
    QMap<QString, int> inDegree;
    for (auto* ni : nodeItems) {
        inDegree[ni->nodeId()] = 0;
    }
    for (auto* conn : m_nodeScene->allConnectionItems()) {
        auto c = conn->toConnection();
        successors[c.sourceNodeId()].append(c.targetNodeId());
        inDegree[c.targetNodeId()]++;
    }

    // Kahn拓扑排序 + 分层
    QMap<QString, int> layer;
    QQueue<QString> queue;
    for (auto it = inDegree.begin(); it != inDegree.end(); ++it) {
        if (it.value() == 0) {
            queue.enqueue(it.key());
            layer[it.key()] = 0;
        }
    }
    // 未连线的节点也加入
    for (auto* ni : nodeItems) {
        if (!layer.contains(ni->nodeId()) && inDegree[ni->nodeId()] == 0)
            queue.enqueue(ni->nodeId());
    }

    int maxLayer = 0;
    while (!queue.isEmpty()) {
        QString cur = queue.dequeue();
        int curLayer = layer[cur];
        for (const auto& succ : successors[cur]) {
            int newLayer = qMax(layer.value(succ, 0), curLayer + 1);
            layer[succ] = newLayer;
            maxLayer = qMax(maxLayer, newLayer);
            inDegree[succ]--;
            if (inDegree[succ] == 0) queue.enqueue(succ);
        }
    }

    // 按层分组
    QMap<int, QList<NodeGraphicsItem*>> layerNodes;
    for (auto* ni : nodeItems) {
        int l = layer.value(ni->nodeId(), 0);
        layerNodes[l].append(ni);
    }

    // 布局：横向排列层，纵向排列同层节点
    const qreal H_SPACING = 320;  // 层间水平间距
    const qreal V_SPACING = 150;  // 同层节点垂直间距
    qreal x = -300, y = -300;
    for (int l = 0; l <= maxLayer || layerNodes.contains(l); ++l) {
        if (!layerNodes.contains(l)) continue;
        auto& list = layerNodes[l];
        qreal layerHeight = list.size() * V_SPACING;
        qreal yStart = y - layerHeight / 2.0;
        for (int i = 0; i < list.size(); ++i) {
            list[i]->setPos(x, yStart + i * V_SPACING);
        }
        x += H_SPACING;
    }

    zoomToFit();
}

void NodeGraphicsView::wheelEvent(QWheelEvent* event)
{
    qreal factor = qPow(1.001, event->angleDelta().y());
    qreal newZoom = qBound(MIN_ZOOM, m_zoomFactor * factor, MAX_ZOOM);
    qreal s = newZoom / m_zoomFactor;
    this->scale(s, s);
    m_zoomFactor = newZoom;
    emit zoomChanged(m_zoomFactor);
    event->accept();
}

void NodeGraphicsView::mousePressEvent(QMouseEvent* event)
{
    // 中键拖动平移
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStartPos = event->pos();
        m_panStartCenter = mapToScene(viewport()->rect().center());
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // Alt+左键平移
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) {
        m_panning = true;
        m_panStartPos = event->pos();
        m_panStartCenter = mapToScene(viewport()->rect().center());
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void NodeGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_panStartPos;
        QPointF newCenter = m_panStartCenter - QPointF(delta) / m_zoomFactor;
        centerOn(newCenter);
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void NodeGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void NodeGraphicsView::contextMenuEvent(QContextMenuEvent* event)
{
    // 将右键菜单事件传递给场景处理
    // QGraphicsView默认会创建自己的context menu，需要覆盖
    event->accept();
    // 场景的contextMenuEvent会通过QGraphicsScene处理
    QGraphicsView::contextMenuEvent(event);
}
