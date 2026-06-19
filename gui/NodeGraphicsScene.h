// gui/NodeGraphicsScene.h
#pragma once

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMap>
#include <QVector>
#include <QTimer>

class NodeBase;
class NodeGraphicsItem;
class PortGraphicsItem;
class ConnectionGraphicsItem;
class WorkflowGraph;
class Connection;

// 画布场景，管理所有图形项的添加、删除和交互
class NodeGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit NodeGraphicsScene(WorkflowGraph* graph, QObject* parent = nullptr);
    ~NodeGraphicsScene() override;

    // 节点操作
    NodeGraphicsItem* addNodeItem(NodeBase* node);
    void removeNodeItem(const QString& nodeId);
    NodeGraphicsItem* nodeItemById(const QString& nodeId) const;

    // 连线操作
    ConnectionGraphicsItem* addConnectionItem(const Connection& conn);
    void removeConnectionItem(const Connection& conn);
    void removeConnectionItem(ConnectionGraphicsItem* item);

    void updateAllConnections();
    void requestConnectionUpdate();

    // 获取所有连线
    QVector<ConnectionGraphicsItem*> allConnectionItems() const;

    // 从图形层面开始连线拖拽（由PortGraphicsItem触发）
    void startConnectionDrag(PortGraphicsItem* sourcePort);
    void updateConnectionDrag(const QPointF& scenePos);
    void finishConnectionDrag(PortGraphicsItem* targetPort);
    void cancelConnectionDrag();

    // 回调（由NodeGraphicsItem触发）
    void onNodeMoved(NodeGraphicsItem* item);
    void onNodeSelected(NodeGraphicsItem* item);
    void onNodeDoubleClicked(NodeGraphicsItem* item);
    void onPreviewRequested(NodeGraphicsItem* item);
    void onSettingsRequested(NodeGraphicsItem* item);

    // 画布背景绘制
    void setGridSize(int size);
    int gridSize() const { return m_gridSize; }

signals:
    void nodeAdded(const QString& nodeId);
    void nodeRemoved(const QString& nodeId);
    void nodeDoubleClicked(const QString& nodeId);
    void previewRequested(const QString& nodeId);
    void settingsRequested(const QString& nodeId);
    void nodeSelected(const QString& nodeId);
    void selectionCleared();
    void connectionDragStarted();
    void connectionDragFinished(bool accepted);
    void statusMessage(const QString& msg);

public slots:
    void onGraphNodeAdded(const QString& nodeId);
    void onGraphNodeRemoved(const QString& nodeId);
    void onGraphConnectionAdded(const Connection& conn);
    void onGraphConnectionRemoved(const Connection& conn);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;

signals:
    void nodeContextMenuRequested(const QString& nodeId, const QPointF& scenePos);
    void canvasContextMenuRequested(const QPointF& scenePos);
    void connectionContextMenuRequested(ConnectionGraphicsItem* conn, const QPointF& scenePos);

private:
    // 在场景位置查找端口图形项
    PortGraphicsItem* findPortAt(const QPointF& scenePos) const;

    // 检查连线兼容性
    bool checkConnectionCompatible(PortGraphicsItem* src, PortGraphicsItem* dst,
                                   QString& errorMsg) const;

    WorkflowGraph* m_graph;

    QMap<QString, NodeGraphicsItem*> m_nodeItems;
    QVector<ConnectionGraphicsItem*> m_connectionItems;

    // 连线拖拽状态
    bool m_connectionDragging = false;
    PortGraphicsItem* m_dragSourcePort = nullptr;
    QGraphicsPathItem* m_tempConnection = nullptr;

    // 自定义框选状态
    bool m_rubberBanding = false;
    QPointF m_rubberBandOrigin;
    QGraphicsRectItem* m_rubberBandRect = nullptr;

    // 连线更新节流
    bool m_connectionsDirty = false;
    QTimer* m_connectionUpdateTimer;

    // 网格
    int m_gridSize = 20;
    QColor m_gridColor = QColor("#2a2a2a");
    QColor m_bgColor = QColor("#1e1e1e");
};
