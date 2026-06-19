#pragma once

#include <QWidget>
#include <QVariantMap>
#include <QVariantList>
#include <QVector>
#include <QPoint>
#include <QColor>

// 网格编辑器 —— 用于 MultiImageLayout 节点
// 支持单元格启用/禁用与合并，输出 rows/cols/cells 结构
class GridEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GridEditor(QWidget *parent = nullptr);

    void setGridConfig(const QVariantMap &config);
    QVariantMap gridConfig() const;

    // 便捷方法：直接设置行列数
    void setGridSize(int rows, int cols);

    QSize sizeHint() const override { return QSize(300, 300); }

signals:
    void valueChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    struct CellInfo {
        bool enabled = true;
        int mergeGroup = -1;   // 合并组 ID，-1 表示未合并
    };

    // 从坐标计算单元格
    QPoint cellAt(const QPointF &pos) const;
    QRect cellRect(int row, int col) const;

    // 合并操作
    void mergeSelectedCells();
    void splitSelectedCells();

    // 获取合并区域（返回该组内所有单元格）
    QVector<QPoint> mergeGroupCells(int groupId) const;

    // 重新分配合并组 ID（保证连续）
    void renumberMergeGroups();

    int m_rows = 3;
    int m_cols = 3;
    QVector<QVector<CellInfo>> m_cells;

    // 选择状态
    QPoint m_selectStart{-1, -1};
    QPoint m_selectEnd{-1, -1};
    bool m_selecting = false;
    int m_hoverRow = -1;
    int m_hoverCol = -1;

    // 颜色表（用于区分合并组）
    static const QVector<QColor> kMergeColors;

    static constexpr int kCellSize = 64;
    static constexpr int kCellSpacing = 3;
    static constexpr int kMargin = 8;
};
