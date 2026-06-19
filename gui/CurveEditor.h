#pragma once

#include <QWidget>
#include <QVector>
#include <QPointF>

// 色调曲线编辑器 —— 用于 Curves 节点
// 用户可在 256x256 区域内编辑控制点，绘制样条曲线
class CurveEditor : public QWidget
{
    Q_OBJECT

public:
    explicit CurveEditor(QWidget *parent = nullptr);

    void setControlPoints(const QVector<QPointF> &points);
    QVector<QPointF> controlPoints() const;

    QSize sizeHint() const override { return QSize(256, 256); }
    QSize minimumSizeHint() const override { return QSize(256, 256); }

signals:
    void valueChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    // 将控件坐标映射到曲线坐标 (0~255)
    QPointF widgetToCurve(const QPointF &wp) const;
    QPointF curveToWidget(const QPointF &cp) const;

    // 搜索距离给定点最近的控制点，返回索引，-1 表示未找到
    int hitTest(const QPointF &wp) const;

    // Catmull-Rom 样条插值
    static QVector<QPointF> computeSpline(const QVector<QPointF> &pts, int segmentSamples = 32);

    // 绘制网格背景
    void drawGrid(QPainter &painter, const QRect &r);

    QVector<QPointF> m_points;
    int m_dragIndex = -1;           // 当前正在拖拽的控制点索引
    bool m_dragging = false;

    static constexpr int kMaxPoints = 10;
    static constexpr double kHitRadius = 8.0;
    static constexpr int kMargin = 4;
};
