#pragma once

#include <QWidget>
#include <QVariantList>
#include <QVector>
#include <QColor>

// 渐变色编辑器 —— 用于 GradientMap 节点
// 水平条显示渐变预览，下方三角形色标可拖拽、双击改色、右键删除
class GradientEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GradientEditor(QWidget *parent = nullptr);

    void setColorStops(const QVariantList &stops);
    QVariantList colorStops() const;

    QSize sizeHint() const override { return QSize(300, 80); }
    QSize minimumSizeHint() const override { return QSize(200, 60); }

signals:
    void valueChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    struct ColorStop {
        double pos = 0.0;    // 0.0 ~ 1.0
        QColor color;
    };

    // 位置 → 像素；像素 → 位置
    double widgetXToPos(int x) const;
    int posToWidgetX(double pos) const;

    // 命中检测
    int hitTest(const QPointF &wp) const;

    // 排序色标（按 pos）
    void sortStops();

    // 绘图区域
    QRect barRect() const;

    QVector<ColorStop> m_stops;
    int m_dragIndex = -1;

    static constexpr int kBarHeight = 30;
    static constexpr int kStopSize = 12;
    static constexpr int kMargin = 12;
};
