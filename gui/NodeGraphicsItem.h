// gui/NodeGraphicsItem.h
#pragma once

#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QVector>
#include "../core/NodeBase.h"

class PortGraphicsItem;
class NodeGraphicsScene;

// 画布上的节点图形卡片
class NodeGraphicsItem : public QGraphicsItem
{
public:
    NodeGraphicsItem(NodeBase* node, QGraphicsItem* parent = nullptr);
    ~NodeGraphicsItem() override;

    NodeBase* node() const { return m_node; }
    QString nodeId() const;

    // 端口图形项
    PortGraphicsItem* inputPortItem(const QString& portName) const;
    PortGraphicsItem* outputPortItem(const QString& portName) const;
    QVector<PortGraphicsItem*> allInputPortItems() const { return m_inputPortItems; }
    QVector<PortGraphicsItem*> allOutputPortItems() const { return m_outputPortItems; }

    // 布局：根据端口数量动态计算卡片尺寸
    void updateLayout();

    // 高亮/选中状态
    void setExecutionState(bool running, bool success, bool failed);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void createPortItems();
    void clearPortItems();

    NodeBase* m_node;

    QVector<PortGraphicsItem*> m_inputPortItems;
    QVector<PortGraphicsItem*> m_outputPortItems;

    // 预览和设置按钮区域（标题栏右侧）
    QRectF m_previewBtnRect;
    QRectF m_settingsBtnRect;

    // 执行状态
    bool m_running = false;
    bool m_success = true;
    bool m_failed = false;

    QPointF m_dragStartPos;
    bool m_dragStarted = false;

    // 尺寸
    static constexpr qreal TITLE_BAR_HEIGHT = 24.0;
    static constexpr qreal PORT_SPACING = 16.0;
    static constexpr qreal MIN_WIDTH = 140.0;
    static constexpr qreal MAX_WIDTH = 280.0;
    static constexpr qreal CORNER_RADIUS = 8.0;
    static constexpr qreal TITLE_PADDING = 8.0;
    static constexpr qreal PORT_AREA_PADDING = 6.0;
    static constexpr qreal TOP_PADDING = 28.0; // title bar + padding

    qreal m_cardWidth = MIN_WIDTH;
    qreal m_cardHeight = 60.0;
};
