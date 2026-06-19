#include "GradientEditor.h"

#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>
#include <QLinearGradient>
#include <QPainterPath>
#include <QToolTip>
#include <QtMath>

GradientEditor::GradientEditor(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);

    // 默认色标：红 → 蓝
    m_stops = {
        { 0.0, QColor(255, 0, 0) },
        { 1.0, QColor(0, 0, 255) },
    };

    setStyleSheet(R"(
        GradientEditor {
            background-color: #2b2b2b;
            border: 1px solid #4a4a4a;
        }
    )");
}

void GradientEditor::setColorStops(const QVariantList &stops)
{
    m_stops.clear();
    for (const QVariant &v : stops) {
        QVariantMap s = v.toMap();
        ColorStop cs;
        cs.pos = qBound(0.0, s.value("pos").toDouble(), 1.0);

        QVariantList c = s.value("color").toList();
        if (c.size() >= 4)
            cs.color = QColor(c[0].toInt(), c[1].toInt(), c[2].toInt(), c[3].toInt());
        else if (c.size() >= 3)
            cs.color = QColor(c[0].toInt(), c[1].toInt(), c[2].toInt());
        else
            cs.color = Qt::white;

        m_stops.append(cs);
    }
    // 确保至少 2 个色标
    if (m_stops.size() < 2) {
        m_stops.clear();
        m_stops.append({0.0, QColor(255, 0, 0)});
        m_stops.append({1.0, QColor(0, 0, 255)});
    }
    sortStops();
    update();
}

QVariantList GradientEditor::colorStops() const
{
    QVariantList result;
    for (const auto &s : m_stops) {
        QVariantMap m;
        m["pos"] = s.pos;
        QVariantList c;
        c.append(s.color.red());
        c.append(s.color.green());
        c.append(s.color.blue());
        c.append(s.color.alpha());
        m["color"] = c;
        result.append(m);
    }
    return result;
}

// ---- 坐标映射 ----

double GradientEditor::widgetXToPos(int x) const
{
    QRect bar = barRect();
    if (bar.width() <= 0) return 0.0;
    return qBound(0.0, double(x - bar.left()) / bar.width(), 1.0);
}

int GradientEditor::posToWidgetX(double pos) const
{
    QRect bar = barRect();
    return bar.left() + qRound(pos * bar.width());
}

QRect GradientEditor::barRect() const
{
    int barWidth = width() - kMargin * 2;
    int barX = kMargin;
    int barY = kMargin + 6; // 上方留空给三角形
    return {barX, barY, barWidth, kBarHeight};
}

int GradientEditor::hitTest(const QPointF &wp) const
{
    int barY = barRect().bottom();
    for (int i = 0; i < m_stops.size(); ++i) {
        int sx = posToWidgetX(m_stops[i].pos);
        int sy = barY + kStopSize / 2;
        double dx = wp.x() - sx;
        double dy = wp.y() - sy;
        if (dx * dx + dy * dy < (kStopSize + 4) * (kStopSize + 4))
            return i;
    }
    return -1;
}

// ---- 排序 ----

void GradientEditor::sortStops()
{
    std::sort(m_stops.begin(), m_stops.end(),
              [](const ColorStop &a, const ColorStop &b) {
                  return a.pos < b.pos;
              });
}

// ---- 绘制 ----

void GradientEditor::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect bar = barRect();

    // 渐变预览条背景（棋盘格表示透明度）
    painter.fillRect(bar, QColor(0x1e, 0x1e, 0x1e));
    // 绘制棋盘格
    int cs = 6;
    for (int y = bar.top(); y < bar.bottom(); y += cs) {
        for (int x = bar.left(); x < bar.right(); x += cs) {
            bool even = ((x - bar.left()) / cs + (y - bar.top()) / cs) % 2 == 0;
            painter.fillRect(x, y, cs, cs,
                             even ? QColor(0x33, 0x33, 0x33)
                                  : QColor(0x55, 0x55, 0x55));
        }
    }

    // 渐变
    QLinearGradient grad(bar.left(), 0, bar.right(), 0);
    // 在端点前后做 clamp，使第一个/最后一个色标的颜色向两端延伸
    grad.setColorAt(0.0, m_stops.first().color);
    for (int i = 1; i < m_stops.size() - 1; ++i) {
        grad.setColorAt(m_stops[i].pos, m_stops[i].color);
    }
    grad.setColorAt(1.0, m_stops.last().color);
    painter.fillRect(bar, grad);

    // 条形边框
    painter.setPen(QPen(QColor(0x4a, 0x4a, 0x4a), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(bar);

    // 绘制色标三角形
    int barBottom = bar.bottom();
    for (int i = 0; i < m_stops.size(); ++i) {
        int sx = posToWidgetX(m_stops[i].pos);
        int sy = barBottom;

        // 三角形
        QPainterPath tri;
        tri.moveTo(sx, sy + kStopSize);
        tri.lineTo(sx - kStopSize / 2, sy);
        tri.lineTo(sx + kStopSize / 2, sy);
        tri.closeSubpath();

        QColor fill = m_stops[i].color;
        if (i == m_dragIndex) {
            fill = fill.lighter(150);
        }

        painter.setBrush(fill);
        painter.setPen(QPen(
            (i == m_dragIndex) ? QColor(0x4E, 0xC9, 0xB0) : QColor(0xe0, 0xe0, 0xe0),
            (i == m_dragIndex) ? 2 : 1));
        painter.drawPath(tri);

        // 选中时的垂直线
        if (i == m_dragIndex) {
            painter.setPen(QPen(QColor(0x4E, 0xC9, 0xB0), 1, Qt::DotLine));
            painter.drawLine(sx, bar.top(), sx, bar.bottom());
        }
    }
}

// ---- 鼠标事件 ----

void GradientEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        int idx = hitTest(event->pos());
        if (idx >= 0 && m_stops.size() > 2) {
            m_stops.removeAt(idx);
            update();
            emit valueChanged();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(event->pos());
        if (idx >= 0) {
            m_dragIndex = idx;
        } else {
            // 在条下方点击：新增色标
            QRect bar = barRect();
            if (event->pos().y() > bar.bottom() - 4 && event->pos().x() >= bar.left() && event->pos().x() <= bar.right()) {
                double pos = widgetXToPos(event->pos().x());
                // 获取该位置在渐变中的颜色
                double t = pos;
                QColor interpColor;
                for (int i = 0; i < m_stops.size() - 1; ++i) {
                    if (t >= m_stops[i].pos && t <= m_stops[i + 1].pos) {
                        double localT = (t - m_stops[i].pos) / (m_stops[i + 1].pos - m_stops[i].pos);
                        interpColor = QColor(
                            qBound(0, m_stops[i].color.red()   + int(localT * (m_stops[i + 1].color.red()   - m_stops[i].color.red())),   255),
                            qBound(0, m_stops[i].color.green() + int(localT * (m_stops[i + 1].color.green() - m_stops[i].color.green())), 255),
                            qBound(0, m_stops[i].color.blue()  + int(localT * (m_stops[i + 1].color.blue()  - m_stops[i].color.blue())),  255),
                            qBound(0, m_stops[i].color.alpha() + int(localT * (m_stops[i + 1].color.alpha() - m_stops[i].color.alpha())), 255)
                        );
                        break;
                    }
                }
                if (!interpColor.isValid())
                    interpColor = m_stops.last().color;

                ColorStop cs{pos, interpColor};
                m_stops.append(cs);
                sortStops();
                // 找到新插入的色标索引
                for (int i = 0; i < m_stops.size(); ++i) {
                    if (qFuzzyCompare(m_stops[i].pos, pos)) {
                        m_dragIndex = i;
                        break;
                    }
                }
                update();
                emit valueChanged();
            }
        }
    }
}

void GradientEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex >= 0 && m_dragIndex < m_stops.size()) {
        QRect bar = barRect();
        double pos = widgetXToPos(event->pos().x());
        m_stops[m_dragIndex].pos = pos;

        // 保持最小值 > 前一个，最大值 < 后一个（拖拽时适当放宽）
        sortStops();
        // 重新找到被拖拽的色标（排序后位置可能变化）
        for (int i = 0; i < m_stops.size(); ++i) {
            if (qFuzzyCompare(m_stops[i].pos, pos)) {
                m_dragIndex = i;
                break;
            }
        }
        // 若位置被截断，更新到实际位置
        if (m_dragIndex >= 0) {
            QRect b = barRect();
            if (event->pos().x() < b.left())
                m_stops[m_dragIndex].pos = 0.0;
            else if (event->pos().x() > b.right())
                m_stops[m_dragIndex].pos = 1.0;
        }

        update();
        emit valueChanged();
    } else {
        int idx = hitTest(event->pos());
        setCursor(idx >= 0 ? Qt::SizeHorCursor : Qt::ArrowCursor);
    }
}

void GradientEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragIndex = -1;
        update();
    }
}

void GradientEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(event->pos());
        if (idx >= 0) {
            QColor newColor = QColorDialog::getColor(m_stops[idx].color, this, "选择色标颜色",
                                                      QColorDialog::ShowAlphaChannel);
            if (newColor.isValid()) {
                m_stops[idx].color = newColor;
                update();
                emit valueChanged();
            }
        }
    }
}
