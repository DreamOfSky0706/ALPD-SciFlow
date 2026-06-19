// gui/NodeGraphicsView.h
#pragma once

#include <QGraphicsView>

class NodeGraphicsScene;

// 画布视图，支持缩放、平移、框选
class NodeGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit NodeGraphicsView(NodeGraphicsScene* scene, QWidget* parent = nullptr);

    // 缩放
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void zoomTo(qreal factor);
    qreal currentZoom() const { return m_zoomFactor; }

    // 自动布局
    void autoLayoutNodes();

signals:
    void zoomChanged(qreal factor);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    NodeGraphicsScene* m_nodeScene;
    qreal m_zoomFactor = 1.0;

    // 平移
    bool m_panning = false;
    QPoint m_panStartPos;
    QPointF m_panStartCenter;

    // 框选
    bool m_rubberBanding = false;
    QPoint m_rubberBandOrigin;

    static constexpr qreal MIN_ZOOM = 0.25;
    static constexpr qreal MAX_ZOOM = 4.0;
};
