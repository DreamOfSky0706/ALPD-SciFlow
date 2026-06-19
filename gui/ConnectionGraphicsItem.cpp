// gui/ConnectionGraphicsItem.cpp
#include "ConnectionGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <cmath>

ConnectionGraphicsItem::ConnectionGraphicsItem(PortGraphicsItem* sourcePort,
                                                 PortGraphicsItem* targetPort,
                                                 QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
    , m_sourcePort(sourcePort)
    , m_targetPort(targetPort)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptHoverEvents(true);
    setZValue(0);

    updatePath();
}

Connection ConnectionGraphicsItem::toConnection() const
{
    return Connection(
        m_sourcePort->parentNodeItem()->nodeId(),
        m_sourcePort->portDef().name,
        m_targetPort->parentNodeItem()->nodeId(),
        m_targetPort->portDef().name
    );
}

void ConnectionGraphicsItem::updatePath()
{
    QPointF p0 = m_sourcePort->centerInScene();
    QPointF p3 = m_targetPort->centerInScene();

    // 在本地坐标中计算
    QPointF localP0 = mapFromScene(p0);
    QPointF localP3 = mapFromScene(p3);

    qreal dx = qAbs(localP3.x() - localP0.x());
    qreal offset = qBound(50.0, dx * 0.4, 150.0);

    // 如果目标在源左侧，增大offset并弯曲
    if (localP3.x() < localP0.x()) {
        offset = qBound(100.0, dx * 0.6, 200.0);
    }

    QPointF c1(localP0.x() + offset, localP0.y());
    QPointF c2(localP3.x() - offset, localP3.y());

    QPainterPath path;
    path.moveTo(localP0);
    path.cubicTo(c1, c2, localP3);
    setPath(path);
}

QRectF ConnectionGraphicsItem::boundingRect() const
{
    return QGraphicsPathItem::boundingRect().adjusted(-5, -5, 5, 5);
}

QPainterPath ConnectionGraphicsItem::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    return stroker.createStroke(path());
}

void ConnectionGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                    QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setRenderHint(QPainter::Antialiasing);

    QColor lineColor = m_sourcePort->portColor();
    if (m_dimmed)
        lineColor.setAlpha(50);

    qreal lineWidth = DEFAULT_LINE_WIDTH;
    if (m_hovered || isSelected()) {
        lineWidth = HOVER_LINE_WIDTH;
        lineColor = lineColor.lighter(150);
    }

    QPen pen(lineColor, lineWidth);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path());

    // 绘制箭头（在目标端）
    if (path().elementCount() > 0) {
        QPointF endPoint = path().currentPosition();
        // 计算切线方向
        qreal t = 0.98;
        QPointF nearEnd = path().pointAtPercent(t);
        QPointF dir = endPoint - nearEnd;
        qreal len = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());
        if (len > 0) {
            dir /= len;

            qreal arrowSize = 6.0;
            QPointF perp(-dir.y(), dir.x());

            QPointF arrowP1 = endPoint - dir * arrowSize * 2 + perp * arrowSize;
            QPointF arrowP2 = endPoint - dir * arrowSize * 2 - perp * arrowSize;

            QPolygonF arrowHead;
            arrowHead << endPoint << arrowP1 << arrowP2;

            painter->setPen(Qt::NoPen);
            painter->setBrush(lineColor);
            painter->drawPolygon(arrowHead);
        }
    }
}

void ConnectionGraphicsItem::setDimmed(bool dimmed)
{
    m_dimmed = dimmed;
    update();
}

void ConnectionGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void ConnectionGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}
