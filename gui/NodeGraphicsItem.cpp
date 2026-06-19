// gui/NodeGraphicsItem.cpp
#include "NodeGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "NodeGraphicsScene.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>
#include <QCursor>

NodeGraphicsItem::NodeGraphicsItem(NodeBase* node, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_node(node)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);
    setCursor(Qt::ArrowCursor);
    setZValue(1);

    createPortItems();
    updateLayout();
    setPos(node->position());
}

NodeGraphicsItem::~NodeGraphicsItem()
{
    // Qt 父子机制自动删除 PortGraphicsItem 子项，只需清空引用
    m_inputPortItems.clear();
    m_outputPortItems.clear();
}

QString NodeGraphicsItem::nodeId() const
{
    return m_node ? m_node->id() : QString();
}

PortGraphicsItem* NodeGraphicsItem::inputPortItem(const QString& portName) const
{
    for (auto* p : m_inputPortItems) {
        if (p->portDef().name == portName)
            return p;
    }
    return nullptr;
}

PortGraphicsItem* NodeGraphicsItem::outputPortItem(const QString& portName) const
{
    for (auto* p : m_outputPortItems) {
        if (p->portDef().name == portName)
            return p;
    }
    return nullptr;
}

void NodeGraphicsItem::createPortItems()
{
    clearPortItems();

    // 创建输入端口图形项
    const auto& inputs = m_node->inputPorts();
    for (int i = 0; i < inputs.size(); ++i) {
        auto* p = new PortGraphicsItem(inputs[i], false, this, this);
        m_inputPortItems.append(p);
    }

    // 创建输出端口图形项
    const auto& outputs = m_node->outputPorts();
    for (int i = 0; i < outputs.size(); ++i) {
        auto* p = new PortGraphicsItem(outputs[i], true, this, this);
        m_outputPortItems.append(p);
    }
}

void NodeGraphicsItem::clearPortItems()
{
    for (auto* p : m_inputPortItems) {
        if (p->scene())
            p->scene()->removeItem(p);
        delete p;
    }
    m_inputPortItems.clear();

    for (auto* p : m_outputPortItems) {
        if (p->scene())
            p->scene()->removeItem(p);
        delete p;
    }
    m_outputPortItems.clear();
}

void NodeGraphicsItem::updateLayout()
{
    // 同步动态端口：若节点端口数多于图形项，追加新端口
    int nodeInCount = m_node->inputPorts().size();
    while (m_inputPortItems.size() < nodeInCount) {
        int i = m_inputPortItems.size();
        auto* p = new PortGraphicsItem(m_node->inputPorts()[i], false, this, this);
        m_inputPortItems.append(p);
    }
    int nodeOutCount = m_node->outputPorts().size();
    while (m_outputPortItems.size() < nodeOutCount) {
        int i = m_outputPortItems.size();
        auto* p = new PortGraphicsItem(m_node->outputPorts()[i], true, this, this);
        m_outputPortItems.append(p);
    }

    int maxPorts = qMax(m_inputPortItems.size(), m_outputPortItems.size());
    qreal portsHeight = maxPorts > 0 ? maxPorts * PORT_SPACING : PORT_SPACING;

    // 计算标题宽度
    QFont titleFont("Microsoft YaHei", 10, QFont::Bold);
    QFontMetrics fm(titleFont);
    qreal titleWidth = fm.horizontalAdvance(m_node->displayName()) + TITLE_PADDING * 2 + 30; // 30 for buttons

    // 计算端口标签宽度
    int labelFontSize = 7;
    QFont labelFont("Microsoft YaHei", labelFontSize);
    QFontMetrics lfm(labelFont);

    qreal maxInputLabelWidth = 0;
    for (auto* p : m_inputPortItems) {
        QString label = QString("%1 (%2)")
            .arg(p->portDef().name)
            .arg(dataTypeAbbrev(p->portDef().dataType));
        maxInputLabelWidth = qMax(maxInputLabelWidth, (qreal)lfm.horizontalAdvance(label));
    }

    qreal maxOutputLabelWidth = 0;
    for (auto* p : m_outputPortItems) {
        QString label = QString("%1 (%2)")
            .arg(p->portDef().name)
            .arg(dataTypeAbbrev(p->portDef().dataType));
        maxOutputLabelWidth = qMax(maxOutputLabelWidth, (qreal)lfm.horizontalAdvance(label));
    }

    // 卡片宽度 = 端口圆点(10) + 间距(4) + 标签 + 间距(8) + 标题 + 间距(8) + 标签 + 间距(4) + 端口圆点(10)
    qreal needWidth = 10 + 4 + maxInputLabelWidth + 8 + titleWidth + 8 + maxOutputLabelWidth + 4 + 10;
    m_cardWidth = qBound(MIN_WIDTH, needWidth, MAX_WIDTH);

    // 卡片高度（+14px留给底部ID行）
    m_cardHeight = TOP_PADDING + portsHeight + PORT_AREA_PADDING * 2 + 14;

    // 布局端口位置
    qreal startY = TOP_PADDING + PORT_AREA_PADDING + PORT_SPACING / 2;
    for (int i = 0; i < m_inputPortItems.size(); ++i) {
        m_inputPortItems[i]->setPos(0, startY + i * PORT_SPACING);
    }
    for (int i = 0; i < m_outputPortItems.size(); ++i) {
        m_outputPortItems[i]->setPos(m_cardWidth, startY + i * PORT_SPACING);
    }

    // 按钮位置
    m_previewBtnRect = QRectF(m_cardWidth - 44, 4, 16, 16);
    m_settingsBtnRect = QRectF(m_cardWidth - 24, 4, 16, 16);

    prepareGeometryChange();
}

QRectF NodeGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, m_cardWidth, m_cardHeight);
}

// 分类底色映射——清新明快的柔和配色
static QColor categoryColor(const QString& categoryPath)
{
    if (categoryPath.startsWith("输入输出"))     return QColor(0x3a, 0x4a, 0x5c); // 天蓝
    if (categoryPath.startsWith("几何变换"))     return QColor(0x3a, 0x50, 0x42); // 薄荷绿
    if (categoryPath.startsWith("色彩调整"))     return QColor(0x4a, 0x3e, 0x58); // 薰衣草紫
    if (categoryPath.startsWith("滤波与模糊"))   return QColor(0x3a, 0x4c, 0x52); // 湖水青
    if (categoryPath.startsWith("形态学操作"))   return QColor(0x50, 0x44, 0x36); // 杏橙
    if (categoryPath.startsWith("边缘检测"))     return QColor(0x50, 0x3c, 0x40); // 豆沙红
    if (categoryPath.startsWith("通道操作"))     return QColor(0x38, 0x48, 0x50); // 蓝绿
    if (categoryPath.startsWith("图层与合成"))   return QColor(0x4c, 0x48, 0x36); // 暖卡其
    if (categoryPath.startsWith("文字与标注"))   return QColor(0x46, 0x3c, 0x52); // 淡紫罗兰
    if (categoryPath.startsWith("数据处理"))     return QColor(0x48, 0x40, 0x34); // 可可棕
    if (categoryPath.startsWith("图表生成"))     return QColor(0x38, 0x3c, 0x50); // 藏蓝
    if (categoryPath.startsWith("风格化"))       return QColor(0x4c, 0x3c, 0x42); // 淡珊瑚
    if (categoryPath.startsWith("版面排版"))     return QColor(0x3c, 0x48, 0x3a); // 鼠尾草绿
    if (categoryPath.startsWith("图形绘制"))     return QColor(0x4a, 0x42, 0x36); // 蜂蜜琥珀
    if (categoryPath.startsWith("实用工具"))     return QColor(0x3e, 0x40, 0x46); // 雾灰
    if (categoryPath.startsWith("几何图案"))     return QColor(0x46, 0x3e, 0x4c); // 淡丁香
    return QColor(0x2d, 0x2d, 0x2d); // 默认深灰
}

void NodeGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                              QWidget* widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    painter->setRenderHint(QPainter::Antialiasing);

    // 阴影
    QRectF shadowRect = boundingRect().adjusted(2, 2, 2, 2);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 60));
    painter->drawRoundedRect(shadowRect, CORNER_RADIUS, CORNER_RADIUS);

    // 卡片主体（按分类着色）
    QRectF bodyRect = boundingRect();
    QColor bodyColor = categoryColor(m_node->categoryPath());

    // 失败时变红
    if (m_failed) {
        bodyColor = QColor(0x4d, 0x2d, 0x2d);
    }
    painter->setBrush(bodyColor);

    // 边框
    QPen borderPen(QColor(0x3d, 0x3d, 0x3d), 1);
    if (isSelected()) {
        borderPen.setColor(QColor("#4EC9B0"));
        borderPen.setWidthF(2.0);
    } else if (m_failed) {
        borderPen.setColor(QColor("#e04040"));
    }
    painter->setPen(borderPen);
    painter->drawRoundedRect(bodyRect, CORNER_RADIUS, CORNER_RADIUS);

    // 标题栏背景
    QPainterPath titlePath;
    titlePath.addRoundedRect(QRectF(0, 0, m_cardWidth, TITLE_BAR_HEIGHT),
                              CORNER_RADIUS, CORNER_RADIUS);
    // 剪掉底部圆角
    titlePath.addRect(0, CORNER_RADIUS, m_cardWidth, TITLE_BAR_HEIGHT - CORNER_RADIUS);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0x38, 0x38, 0x38));
    painter->drawPath(titlePath);

    // 标题栏底部线
    painter->setPen(QPen(QColor(0x4a, 0x4a, 0x4a), 1));
    painter->drawLine(QPointF(0, TITLE_BAR_HEIGHT), QPointF(m_cardWidth, TITLE_BAR_HEIGHT));

    // 节点名称
    QFont titleFont("Microsoft YaHei", 10, QFont::Bold);
    painter->setFont(titleFont);
    painter->setPen(QColor("#e0e0e0"));
    painter->drawText(QRectF(TITLE_PADDING, 0, m_cardWidth - TITLE_PADDING * 2 - 50, TITLE_BAR_HEIGHT),
                      Qt::AlignLeft | Qt::AlignVCenter, m_node->displayName());

    // 预览按钮（眼睛图标）
    QPen iconPen(QColor("#a0a0a0"), 1.2);
    painter->setPen(iconPen);
    painter->setBrush(Qt::NoBrush);
    // 简化的眼睛图标
    painter->drawEllipse(m_previewBtnRect.adjusted(2, 4, -2, -4));
    painter->drawEllipse(m_previewBtnRect.center().x() - 1.5, m_previewBtnRect.center().y() - 1.5, 3, 3);

    // 齿轮图标
    painter->drawEllipse(m_settingsBtnRect.adjusted(3, 3, -3, -3));
    QPointF gc = m_settingsBtnRect.center();
    painter->drawLine(QLineF(gc, gc + QPointF(3, -3)));
    painter->drawLine(QLineF(gc, gc + QPointF(4, 0)));
    painter->drawLine(QLineF(gc, gc + QPointF(-1, 4)));

    // 左下角节点ID
    QFont idFont("Consolas", 7);
    painter->setFont(idFont);
    painter->setPen(QColor("#606060"));
    painter->drawText(QRectF(4, m_cardHeight - 14, m_cardWidth - 8, 12),
                      Qt::AlignLeft | Qt::AlignBottom, m_node->id());
}

void NodeGraphicsItem::setExecutionState(bool running, bool success, bool failed)
{
    m_running = running;
    m_success = success;
    m_failed = failed;
    update();
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        if (m_node && scene()) {
            m_node->setPosition(pos());
        }
        // 节流更新连线：每16ms最多更新一次
        if (auto* ns = qobject_cast<NodeGraphicsScene*>(scene())) {
            ns->requestConnectionUpdate();
        }
    }
    if (change == ItemSelectedHasChanged && value.toBool()) {
        if (auto* ns = qobject_cast<NodeGraphicsScene*>(scene())) {
            ns->onNodeSelected(this);
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    // 仅当未移动时响应双击（拖拽后的意外双击不触发）
    if (m_dragStarted) return;
    Q_UNUSED(event)
    if (auto* ns = qobject_cast<NodeGraphicsScene*>(scene())) {
        ns->onNodeDoubleClicked(this);
    }
}

void NodeGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStarted = false;
        m_dragStartPos = pos();

        // 检查是否点击了按钮区域
        QPointF localPos = event->pos();
        if (m_previewBtnRect.contains(localPos)) {
            if (auto* ns = qobject_cast<NodeGraphicsScene*>(scene())) {
                ns->onPreviewRequested(this);
            }
            event->accept();
            return;
        }
        if (m_settingsBtnRect.contains(localPos)) {
            if (auto* ns = qobject_cast<NodeGraphicsScene*>(scene())) {
                ns->onSettingsRequested(this);
            }
            event->accept();
            return;
        }
    }
    if (event->button() == Qt::RightButton) {
        event->accept();
        return;
    }
    QGraphicsItem::mousePressEvent(event);
}

void NodeGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    // 移动超过阈值才标为拖拽中
    if (!m_dragStarted) {
        QPointF delta = event->scenePos() - event->lastScenePos();
        if (delta.manhattanLength() > 2.0)
            m_dragStarted = true;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void NodeGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    m_dragStarted = false;
    // 吸附到20px网格
    if (!(event->modifiers() & Qt::ShiftModifier)) {
        QPointF p = pos();
        p.setX(qRound(p.x() / 20.0) * 20.0);
        p.setY(qRound(p.y() / 20.0) * 20.0);
        setPos(p);
    }
}
