#include "GridEditor.h"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QSet>

// 合并组颜色表（最多 16 种颜色，循环使用）
const QVector<QColor> GridEditor::kMergeColors = {
    QColor(0x4E, 0xC9, 0xB0),
    QColor(0x56, 0x9C, 0xD6),
    QColor(0xD7, 0xBA, 0x7D),
    QColor(0xC5, 0x86, 0xC0),
    QColor(0x9C, 0xD6, 0x4E),
    QColor(0xD6, 0x4E, 0x4E),
    QColor(0x4E, 0xD6, 0xD6),
    QColor(0xD6, 0xA5, 0x4E),
    QColor(0x8E, 0x4E, 0xD6),
    QColor(0x4E, 0xD6, 0x8E),
    QColor(0xD6, 0x4E, 0xA5),
    QColor(0xA5, 0xD6, 0x4E),
    QColor(0x4E, 0x8E, 0xD6),
    QColor(0xD6, 0xD6, 0x4E),
    QColor(0x4E, 0xD6, 0x4E),
    QColor(0xD6, 0x8E, 0x4E),
};

GridEditor::GridEditor(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setMinimumSize(260, 260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 初始化为 3x3 全部启用的网格
    m_cells.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_cells[r].resize(m_cols);
        for (int c = 0; c < m_cols; ++c) {
            m_cells[r][c].enabled = true;
            m_cells[r][c].mergeGroup = -1;
        }
    }

    setStyleSheet(R"(
        GridEditor {
            background-color: #2b2b2b;
            border: 1px solid #4a4a4a;
        }
    )");
}

void GridEditor::setGridSize(int rows, int cols)
{
    rows = qBound(1, rows, 4);
    cols = qBound(1, cols, 4);

    QVector<QVector<CellInfo>> oldCells = m_cells;
    int oldRows = m_rows;
    int oldCols = m_cols;

    m_rows = rows;
    m_cols = cols;
    m_cells.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_cells[r].resize(m_cols);
        for (int c = 0; c < m_cols; ++c) {
            if (r < oldRows && c < oldCols)
                m_cells[r][c] = oldCells[r][c];
            else {
                m_cells[r][c].enabled = true;
                m_cells[r][c].mergeGroup = -1;
            }
        }
    }
    renumberMergeGroups();
    update();
}

void GridEditor::setGridConfig(const QVariantMap &config)
{
    int rows = config.value("rows").toInt();
    int cols = config.value("cols").toInt();
    rows = qBound(1, rows, 4);
    cols = qBound(1, cols, 4);

    m_rows = rows;
    m_cols = cols;
    m_cells.resize(m_rows);
    for (int r = 0; r < m_rows; ++r)
        m_cells[r].resize(m_cols);

    QVariantList cellsList = config.value("cells").toList();
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            int idx = r * m_cols + c;
            if (idx < cellsList.size()) {
                QVariantMap cellData = cellsList[idx].toMap();
                m_cells[r][c].enabled = cellData.value("enabled", true).toBool();
                m_cells[r][c].mergeGroup = cellData.value("mergeGroup", -1).toInt();
            }
        }
    }
    renumberMergeGroups();
    update();
}

QVariantMap GridEditor::gridConfig() const
{
    QVariantMap config;
    config["rows"] = m_rows;
    config["cols"] = m_cols;

    QVariantList cellsList;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            QVariantMap cellData;
            cellData["enabled"] = m_cells[r][c].enabled;
            cellData["mergeGroup"] = m_cells[r][c].mergeGroup;
            cellData["row"] = r;
            cellData["col"] = c;
            cellsList.append(cellData);
        }
    }
    config["cells"] = cellsList;
    return config;
}

// ---- 几何计算 ----

QPoint GridEditor::cellAt(const QPointF &pos) const
{
    int totalW = m_cols * kCellSize + (m_cols - 1) * kCellSpacing + kMargin * 2;
    int totalH = m_rows * kCellSize + (m_rows - 1) * kCellSpacing + kMargin * 2;
    int offsetX = kMargin;  // 左对齐
    int offsetY = kMargin;  // 顶对齐

    int rx = int(pos.x()) - offsetX;
    int ry = int(pos.y()) - offsetY;
    if (rx < 0 || ry < 0) return {-1, -1};

    int cellW = kCellSize + kCellSpacing;
    int col = rx / cellW;
    int row = ry / cellW;
    int xInCell = rx % cellW;
    int yInCell = ry % cellW;

    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return {-1, -1};
    if (xInCell < 0 || xInCell >= kCellSize || yInCell < 0 || yInCell >= kCellSize) return {-1, -1};
    return {row, col};
}

QRect GridEditor::cellRect(int row, int col) const
{
    int offsetX = kMargin;  // 左对齐
    int offsetY = kMargin;  // 顶对齐

    int x = offsetX + col * (kCellSize + kCellSpacing);
    int y = offsetY + row * (kCellSize + kCellSpacing);
    return {x, y, kCellSize, kCellSize};
}

// ---- 合并操作 ----

QVector<QPoint> GridEditor::mergeGroupCells(int groupId) const
{
    QVector<QPoint> cells;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (m_cells[r][c].mergeGroup == groupId)
                cells.append({r, c});
    return cells;
}

void GridEditor::renumberMergeGroups()
{
    // 收集所有正在使用的 group ID
    QMap<int, QVector<QPoint>> groups;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            if (m_cells[r][c].mergeGroup >= 0)
                groups[m_cells[r][c].mergeGroup].append({r, c});

    // 重新编号
    int nextId = 0;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        for (auto &pt : it.value())
            m_cells[pt.x()][pt.y()].mergeGroup = nextId;
        ++nextId;
    }
}

void GridEditor::mergeSelectedCells()
{
    QVector<QPoint> selected;
    int r1 = qMin(m_selectStart.x(), m_selectEnd.x());
    int r2 = qMax(m_selectStart.x(), m_selectEnd.x());
    int c1 = qMin(m_selectStart.y(), m_selectEnd.y());
    int c2 = qMax(m_selectStart.y(), m_selectEnd.y());

    for (int r = r1; r <= r2; ++r)
        for (int c = c1; c <= c2; ++c)
            if (m_cells[r][c].enabled)
                selected.append({r, c});

    if (selected.size() < 2)
        return;

    // 找到一个新的 mergeGroup ID
    int maxGroup = -1;
    for (int r = 0; r < m_rows; ++r)
        for (int c = 0; c < m_cols; ++c)
            maxGroup = qMax(maxGroup, m_cells[r][c].mergeGroup);
    int newGroup = maxGroup + 1;

    // 先清除选中区域内已有的合并组（将这些组的所有单元格一并纳入）
    QSet<int> affectedGroups;
    for (auto &pt : selected) {
        int g = m_cells[pt.x()][pt.y()].mergeGroup;
        if (g >= 0)
            affectedGroups.insert(g);
    }
    // 将受影响组的所有单元格加入 selected
    for (int g : affectedGroups) {
        for (int r = 0; r < m_rows; ++r)
            for (int c = 0; c < m_cols; ++c)
                if (m_cells[r][c].mergeGroup == g) {
                    QPoint pt(r, c);
                    if (!selected.contains(pt))
                        selected.append(pt);
                }
    }

    // 分配新的合并组 ID
    for (auto &pt : selected)
        m_cells[pt.x()][pt.y()].mergeGroup = newGroup;

    renumberMergeGroups();
    update();
    emit valueChanged();
}

void GridEditor::splitSelectedCells()
{
    int r1 = qMin(m_selectStart.x(), m_selectEnd.x());
    int r2 = qMax(m_selectStart.x(), m_selectEnd.x());
    int c1 = qMin(m_selectStart.y(), m_selectEnd.y());
    int c2 = qMax(m_selectStart.y(), m_selectEnd.y());

    bool anyChanged = false;
    for (int r = r1; r <= r2; ++r) {
        for (int c = c1; c <= c2; ++c) {
            if (m_cells[r][c].mergeGroup >= 0) {
                m_cells[r][c].mergeGroup = -1;
                anyChanged = true;
            }
        }
    }

    if (anyChanged) {
        renumberMergeGroups();
        update();
        emit valueChanged();
    }
}

// ---- 绘制 ----

void GridEditor::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            QRect rc = cellRect(r, c);
            const CellInfo &cell = m_cells[r][c];

            // 背景色
            QColor bg;
            if (!cell.enabled) {
                bg = QColor(0x3a, 0x3a, 0x3a);
            } else if (cell.mergeGroup >= 0) {
                int colorIdx = cell.mergeGroup % kMergeColors.size();
                bg = kMergeColors[colorIdx];
                bg.setAlpha(120);
            } else {
                bg = QColor(0x1e, 0x1e, 0x1e);
            }

            painter.setBrush(bg);

            // 选中高亮边框
            bool inSelection = false;
            if (m_selectStart.x() >= 0 && m_selectEnd.x() >= 0) {
                int r1 = qMin(m_selectStart.x(), m_selectEnd.x());
                int r2 = qMax(m_selectStart.x(), m_selectEnd.x());
                int c1 = qMin(m_selectStart.y(), m_selectEnd.y());
                int c2 = qMax(m_selectStart.y(), m_selectEnd.y());
                inSelection = (r >= r1 && r <= r2 && c >= c1 && c <= c2);
            }

            if (inSelection)
                painter.setPen(QPen(QColor(0x4E, 0xC9, 0xB0), 2));
            else
                painter.setPen(QPen(QColor(0x4a, 0x4a, 0x4a), 1));

            painter.drawRect(rc);

            // 文本标签
            painter.setPen(QColor(0xe0, 0xe0, 0xe0));
            QString label;
            if (cell.mergeGroup >= 0)
                label = QString("M%1").arg(cell.mergeGroup + 1);
            else if (cell.enabled)
                label = QString("(%1,%2)").arg(r).arg(c);
            else
                label = "OFF";
            painter.drawText(rc, Qt::AlignCenter, label);
        }
    }

    // 悬停高亮
    if (m_hoverRow >= 0 && m_hoverCol >= 0) {
        QRect rc = cellRect(m_hoverRow, m_hoverCol);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(0x4E, 0xC9, 0xB0), 1, Qt::DashLine));
        painter.drawRect(rc);
    }
}

// ---- 鼠标事件 ----

void GridEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint cell = cellAt(event->pos());
        if (cell.x() >= 0) {
            // 点击切换启用/禁用
            m_cells[cell.x()][cell.y()].enabled = !m_cells[cell.x()][cell.y()].enabled;
            // 如果禁用，清除合并组
            if (!m_cells[cell.x()][cell.y()].enabled)
                m_cells[cell.x()][cell.y()].mergeGroup = -1;
            update();
            emit valueChanged();

            // 开始选择拖拽
            m_selectStart = cell;
            m_selectEnd = cell;
            m_selecting = true;
        }
    }
}

void GridEditor::mouseMoveEvent(QMouseEvent *event)
{
    QPoint cell = cellAt(event->pos());
    m_hoverRow = cell.x();
    m_hoverCol = cell.y();

    if (m_selecting && cell.x() >= 0) {
        m_selectEnd = cell;
        update();
    } else {
        update();
    }
}

void GridEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_selecting = false;
    }
}

void GridEditor::contextMenuEvent(QContextMenuEvent *event)
{
    // 确保有有效选择
    if (m_selectStart.x() < 0) {
        QPoint cell = cellAt(event->pos());
        if (cell.x() >= 0) {
            m_selectStart = cell;
            m_selectEnd = cell;
        }
    }

    QMenu menu(this);

    // 暗色菜单样式
    menu.setStyleSheet(R"(
        QMenu {
            background-color: #2b2b2b;
            color: #e0e0e0;
            border: 1px solid #4a4a4a;
        }
        QMenu::item:selected {
            background-color: #4EC9B0;
            color: #1e1e1e;
        }
    )");

    QAction *mergeAction = menu.addAction("Merge");
    QAction *splitAction = menu.addAction("Split");

    QAction *chosen = menu.exec(event->globalPos());
    if (chosen == mergeAction) {
        mergeSelectedCells();
    } else if (chosen == splitAction) {
        splitSelectedCells();
    }
}
