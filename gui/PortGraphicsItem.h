// gui/PortGraphicsItem.h
#pragma once

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include "../common/PortDefinition.h"

class NodeGraphicsItem;

// 端口小圆点图形项，嵌套在NodeGraphicsItem中
class PortGraphicsItem : public QGraphicsEllipseItem
{
public:
    PortGraphicsItem(const PortDefinition& def, bool isOutput,
                     NodeGraphicsItem* parentNode, QGraphicsItem* parent = nullptr);
    ~PortGraphicsItem() override = default;

    const PortDefinition& portDef() const { return m_def; }
    bool isOutput() const { return m_isOutput; }
    NodeGraphicsItem* parentNodeItem() const { return m_parentNode; }

    // 端口中心在场景中的坐标
    QPointF centerInScene() const;

    // 高亮状态
    void setHighlighted(bool hl);
    void setRejected(bool rej);  // 不兼容时显示红色叉号
    void resetAppearance();

    // 颜色
    QColor portColor() const;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    PortDefinition m_def;
    bool m_isOutput;
    NodeGraphicsItem* m_parentNode;

    QGraphicsTextItem* m_labelText;     // 端口名称 + 类型缩写
    bool m_highlighted = false;
    bool m_rejected = false;
    QPointF m_dragStartPos;
    bool m_dragging = false;

    static constexpr qreal PORT_RADIUS = 5.0;
    static constexpr qreal PORT_DIAMETER = 10.0;
};
