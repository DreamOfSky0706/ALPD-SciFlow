#include "CurveEditor.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QtMath>

CurveEditor::CurveEditor(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    // 默认控制点：左下 (0,0)→ 右上 (255,255)
    m_points = { QPointF(0.0, 0.0), QPointF(255.0, 255.0) };

    setStyleSheet(R"(
        CurveEditor {
            background-color: #2b2b2b;
            border: 1px solid #4a4a4a;
        }
    )");
}

void CurveEditor::setControlPoints(const QVector<QPointF> &points)
{
    m_points = points;
    // 保证至少有两个点
    if (m_points.size() < 2) {
        m_points.clear();
        m_points.append(QPointF(0.0, 0.0));
        m_points.append(QPointF(255.0, 255.0));
    }
    // 限制最多 10 个点
    if (m_points.size() > kMaxPoints)
        m_points.resize(kMaxPoints);
    update();
}

QVector<QPointF> CurveEditor::controlPoints() const
{
    return m_points;
}

// ---- 坐标映射 ----

QPointF CurveEditor::widgetToCurve(const QPointF &wp) const
{
    QRect r = rect().adjusted(kMargin, kMargin, -kMargin, -kMargin);
    double x = (wp.x() - r.left()) / r.width()  * 255.0;
    double y = (r.bottom() - wp.y()) / r.height() * 255.0; // Y 翻转
    x = qBound(0.0, x, 255.0);
    y = qBound(0.0, y, 255.0);
    return {x, y};
}

QPointF CurveEditor::curveToWidget(const QPointF &cp) const
{
    QRect r = rect().adjusted(kMargin, kMargin, -kMargin, -kMargin);
    double wx = cp.x() / 255.0 * r.width()  + r.left();
    double wy = r.bottom() - cp.y() / 255.0 * r.height(); // Y 翻转
    return {wx, wy};
}

int CurveEditor::hitTest(const QPointF &wp) const
{
    int best = -1;
    double bestDist = kHitRadius * kHitRadius;
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF pw = curveToWidget(m_points[i]);
        double dx = wp.x() - pw.x();
        double dy = wp.y() - pw.y();
        double d2 = dx * dx + dy * dy;
        if (d2 < bestDist) {
            bestDist = d2;
            best = i;
        }
    }
    return best;
}

// ---- Catmull-Rom 样条 ----

QVector<QPointF> CurveEditor::computeSpline(const QVector<QPointF> &pts,
                                             int segmentSamples)
{
    QVector<QPointF> result;
    if (pts.size() < 2)
        return result;

    int n = pts.size();
    for (int i = 0; i < n - 1; ++i) {
        QPointF p0 = pts[qMax(0, i - 1)];
        QPointF p1 = pts[i];
        QPointF p2 = pts[i + 1];
        QPointF p3 = pts[qMin(n - 1, i + 2)];

        for (int s = 0; s < segmentSamples; ++s) {
            double t = double(s) / segmentSamples;
            double t2 = t * t;
            double t3 = t2 * t;

            // Catmull-Rom 矩阵形式
            double x = 0.5 * (
                (2.0 * p1.x()) +
                (-p0.x() + p2.x()) * t +
                (2.0 * p0.x() - 5.0 * p1.x() + 4.0 * p2.x() - p3.x()) * t2 +
                (-p0.x() + 3.0 * p1.x() - 3.0 * p2.x() + p3.x()) * t3
            );
            double y = 0.5 * (
                (2.0 * p1.y()) +
                (-p0.y() + p2.y()) * t +
                (2.0 * p0.y() - 5.0 * p1.y() + 4.0 * p2.y() - p3.y()) * t2 +
                (-p0.y() + 3.0 * p1.y() - 3.0 * p2.y() + p3.y()) * t3
            );
            result.append(QPointF(x, y));
        }
    }
    // 添加最后一个点
    result.append(pts.last());
    return result;
}

// ---- 绘制 ----

void CurveEditor::drawGrid(QPainter &painter, const QRect &r)
{
    painter.save();
    QPen gridPen(QColor(0x4a, 0x4a, 0x4a), 1, Qt::DotLine);
    painter.setPen(gridPen);

    // 4x4 网格
    for (int i = 1; i < 4; ++i) {
        double fx = i / 4.0;
        double fy = i / 4.0;
        int x = r.left() + int(r.width()  * fx);
        int y = r.top()  + int(r.height() * fy);
        painter.drawLine(x, r.top(), x, r.bottom());
        painter.drawLine(r.left(), y, r.right(), y);
    }
    painter.restore();
}

void CurveEditor::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect().adjusted(kMargin, kMargin, -kMargin, -kMargin);

    // 绘图区域背景
    painter.fillRect(r, QColor(0x1e, 0x1e, 0x1e));

    // 网格
    drawGrid(painter, r);

    // 外框
    painter.setPen(QPen(QColor(0x4a, 0x4a, 0x4a), 1));
    painter.drawRect(r);

    // 对角线参考线
    painter.setPen(QPen(QColor(0x5a, 0x5a, 0x5a), 1, Qt::DashLine));
    painter.drawLine(r.bottomLeft(), r.topRight());

    // 绘制样条曲线
    if (m_points.size() >= 2) {
        QVector<QPointF> spline = computeSpline(m_points, 16);
        QPainterPath path;
        if (!spline.isEmpty()) {
            QPointF wp0 = curveToWidget(spline[0]);
            path.moveTo(wp0);
            for (int i = 1; i < spline.size(); ++i) {
                QPointF wp = curveToWidget(spline[i]);
                path.lineTo(wp);
            }
            painter.setPen(QPen(QColor(0x4E, 0xC9, 0xB0), 2));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
    }

    // 绘制控制点
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF pw = curveToWidget(m_points[i]);
        QRectF dot(pw.x() - 5, pw.y() - 5, 10, 10);

        // 填充色
        QColor fill = (i == m_dragIndex) ? QColor(0x4E, 0xC9, 0xB0)
                                          : QColor(0xe0, 0xe0, 0xe0);
        // 端点特殊颜色
        if (i == 0 || i == m_points.size() - 1)
            fill = (i == m_dragIndex) ? QColor(0x4E, 0xC9, 0xB0)
                                       : QColor(0xff, 0xa5, 0x00);

        painter.setBrush(fill);
        painter.setPen(QPen(QColor(0x2b, 0x2b, 0x2b), 1.5));
        painter.drawEllipse(dot);
    }
}

// ---- 鼠标事件 ----

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // 右键删除（不能删除端点）
        int idx = hitTest(event->pos());
        if (idx > 0 && idx < m_points.size() - 1 && m_points.size() > 2) {
            m_points.removeAt(idx);
            update();
            emit valueChanged();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(event->pos());
        if (idx >= 0) {
            // 拖拽已有控制点
            m_dragIndex = idx;
            m_dragging = true;
        } else if (m_points.size() < kMaxPoints) {
            // 在空白区域添加新控制点
            QPointF cp = widgetToCurve(event->pos());
            // 按 X 排序插入
            int insertIdx = m_points.size();
            for (int i = 0; i < m_points.size(); ++i) {
                if (cp.x() < m_points[i].x()) {
                    insertIdx = i;
                    break;
                }
            }
            m_points.insert(insertIdx, cp);
            m_dragIndex = insertIdx;
            m_dragging = true;
            update();
            emit valueChanged();
        }
    }
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && m_dragIndex >= 0 && m_dragIndex < m_points.size()) {
        QPointF cp = widgetToCurve(event->pos());

        // 端点：只允许沿边界移动
        if (m_dragIndex == 0) {
            cp.setX(0.0);
        } else if (m_dragIndex == m_points.size() - 1) {
            cp.setX(255.0);
        }

        // 保持 X 有序（非端点不能穿过相邻点）
        if (m_dragIndex > 0 && cp.x() <= m_points[m_dragIndex - 1].x())
            cp.setX(m_points[m_dragIndex - 1].x() + 0.5);
        if (m_dragIndex < m_points.size() - 1 && cp.x() >= m_points[m_dragIndex + 1].x())
            cp.setX(m_points[m_dragIndex + 1].x() - 0.5);

        m_points[m_dragIndex] = cp;
        update();
        emit valueChanged();
    } else {
        // 更新光标
        int idx = hitTest(event->pos());
        setCursor(idx >= 0 ? Qt::PointingHandCursor : Qt::CrossCursor);
    }
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragIndex = -1;
        m_dragging = false;
        update();
    }
}

void CurveEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 双击重置为默认
    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(event->pos());
        if (idx > 0 && idx < m_points.size() - 1) {
            m_points.removeAt(idx);
            update();
            emit valueChanged();
        }
    }
}
