// gui/PortGraphicsItem.cpp
#include "PortGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QCursor>
#include <QApplication>

PortGraphicsItem::PortGraphicsItem(const PortDefinition& def, bool isOutput,
                                     NodeGraphicsItem* parentNode, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-PORT_RADIUS, -PORT_RADIUS, PORT_DIAMETER, PORT_DIAMETER, parent)
    , m_def(def)
    , m_isOutput(isOutput)
    , m_parentNode(parentNode)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setCursor(Qt::CrossCursor);
    setZValue(2);

    // 工具提示
    if (m_isOutput) {
        setToolTip(QString("%1 → %2").arg(m_def.name).arg(dataTypeName(m_def.dataType)));
    } else {
        QStringList acc;
        for (auto t : m_def.acceptedTypes) acc << dataTypeName(t);
        setToolTip(QString("%1 ← 接受: %2").arg(m_def.name).arg(acc.join(", ")));
    }

    QColor c = portColor();
    setBrush(QBrush(c));
    setPen(QPen(c.darker(130), 1.5));

    // 端口标签
    QString label = QString("%1 (%2)")
        .arg(m_def.name)
        .arg(dataTypeAbbrev(m_def.dataType));
    m_labelText = new QGraphicsTextItem(label, this);
    m_labelText->setDefaultTextColor(QColor("#a0a0a0"));
    QFont font("Microsoft YaHei", 7);
    m_labelText->setFont(font);

    // 位置偏移：输入在左，输出在右
    if (m_isOutput) {
        m_labelText->setPos(PORT_RADIUS + 4, -8);
    } else {
        // 左对齐到右侧
        qreal textWidth = m_labelText->boundingRect().width();
        m_labelText->setPos(-PORT_RADIUS - textWidth - 4, -8);
    }

    // 端口标签已显示类型缩写在名称旁，无需悬浮提示（避免Windows下弹出tooltip窗口）
}

QPointF PortGraphicsItem::centerInScene() const
{
    return mapToScene(QPointF(0, 0));
}

QColor PortGraphicsItem::portColor() const
{
    QString hex = dataTypeColor(m_def.dataType);
    return QColor(hex);
}

void PortGraphicsItem::setHighlighted(bool hl)
{
    m_highlighted = hl;
    if (hl) {
        setScale(1.5);
        QColor c = portColor().lighter(150);
        setBrush(QBrush(c));
        setPen(QPen(c, 2.0));
    } else {
        resetAppearance();
    }
}

void PortGraphicsItem::setRejected(bool rej)
{
    m_rejected = rej;
    if (rej) {
        setScale(1.3);
        setBrush(QBrush(QColor("#e04040")));
        setPen(QPen(QColor("#ff0000"), 2.0));
    } else {
        resetAppearance();
    }
}

void PortGraphicsItem::resetAppearance()
{
    m_highlighted = false;
    m_rejected = false;
    setScale(1.0);
    QColor c = portColor();
    setBrush(QBrush(c));
    setPen(QPen(c.darker(130), 1.5));
}

void PortGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    setHighlighted(true);
}

void PortGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event)
    if (!m_dragging)
        resetAppearance();
}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragStartPos = event->scenePos();
        event->accept();
    }
}

void PortGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dragging) {
        // 通知Scene绘制临时连线
        // Scene通过重写mouseMoveEvent来处理
        event->accept();
    }
}

void PortGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_dragging) {
        m_dragging = false;
        resetAppearance();
        // 通知Scene完成连线
        event->accept();
    }
}
