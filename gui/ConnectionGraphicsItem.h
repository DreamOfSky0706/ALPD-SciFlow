// gui/ConnectionGraphicsItem.h
#pragma once

#include <QGraphicsPathItem>
#include "../core/Connection.h"

class PortGraphicsItem;

// 贝塞尔曲线连线图形项
class ConnectionGraphicsItem : public QGraphicsPathItem
{
public:
    ConnectionGraphicsItem(PortGraphicsItem* sourcePort, PortGraphicsItem* targetPort,
                           QGraphicsItem* parent = nullptr);
    ~ConnectionGraphicsItem() override = default;

    PortGraphicsItem* sourcePortItem() const { return m_sourcePort; }
    PortGraphicsItem* targetPortItem() const { return m_targetPort; }

    // 获取对应的逻辑连线对象
    Connection toConnection() const;

    // 当端口位置变化时更新路径
    void updatePath();

    // 设置是否高亮（选中节点关联时）
    void setDimmed(bool dimmed);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    PortGraphicsItem* m_sourcePort;
    PortGraphicsItem* m_targetPort;

    bool m_dimmed = false;
    bool m_hovered = false;

    static constexpr qreal DEFAULT_LINE_WIDTH = 2.0;
    static constexpr qreal HOVER_LINE_WIDTH = 3.0;
};
