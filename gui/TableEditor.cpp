#include "TableEditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QPushButton>
#include <QContextMenuEvent>

TableEditor::TableEditor(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_table = new QTableWidget(0, 0, this);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_table, &QTableWidget::customContextMenuRequested,
            this, [this](const QPoint &) { /* handled via contextMenuEvent override on widget */ });
    // Use cellChanged to emit valueChanged
    connect(m_table, &QTableWidget::cellChanged,
            this, &TableEditor::onCellChanged);

    m_table->setColumnCount(1);
    m_table->setRowCount(1);
    m_table->setHorizontalHeaderLabels({"A"});
    layout->addWidget(m_table, 1);  // 表格占主要空间

    // 底部操作按钮行
    auto* btnRow = new QHBoxLayout;
    auto* addRowBtn = new QPushButton("+行"); addRowBtn->setToolTip("添加一行");
    auto* delRowBtn = new QPushButton("-行"); delRowBtn->setToolTip("删除选中行");
    auto* addColBtn = new QPushButton("+列"); addColBtn->setToolTip("添加一列");
    auto* delColBtn = new QPushButton("-列"); delColBtn->setToolTip("删除选中列");
    QString btnStyle="QPushButton{background:#3a3a3a;color:#e0e0e0;border:1px solid #4a4a4a;padding:2px 8px;}QPushButton:hover{background:#4a4a4a;}";
    addRowBtn->setStyleSheet(btnStyle); delRowBtn->setStyleSheet(btnStyle);
    addColBtn->setStyleSheet(btnStyle); delColBtn->setStyleSheet(btnStyle);
    connect(addRowBtn, &QPushButton::clicked, this, &TableEditor::onAddRow);
    connect(delRowBtn, &QPushButton::clicked, this, &TableEditor::onRemoveRow);
    connect(addColBtn, &QPushButton::clicked, this, &TableEditor::onAddColumn);
    connect(delColBtn, &QPushButton::clicked, this, &TableEditor::onRemoveColumn);
    btnRow->addWidget(addRowBtn); btnRow->addWidget(delRowBtn);
    btnRow->addStretch();
    btnRow->addWidget(addColBtn); btnRow->addWidget(delColBtn);
    layout->addLayout(btnRow);

    setStyleSheet(R"(
        TableEditor {
            background-color: #2b2b2b;
            border: 1px solid #4a4a4a;
        }
        QTableWidget {
            background-color: #2b2b2b;
            color: #e0e0e0;
            gridline-color: #4a4a4a;
            border: 1px solid #4a4a4a;
            selection-background-color: #4EC9B0;
            selection-color: #1e1e1e;
        }
        QTableWidget::item {
            padding: 2px 6px;
        }
        QHeaderView::section {
            background-color: #3a3a3a;
            color: #e0e0e0;
            padding: 4px;
            border: 1px solid #4a4a4a;
            font-weight: bold;
        }
        QTableWidget QTableCornerButton::section {
            background-color: #3a3a3a;
            border: 1px solid #4a4a4a;
        }
    )");
}

void TableEditor::setupTable()
{
    m_table->setColumnCount(3);
    m_table->setRowCount(3);
    QStringList headers = {"A", "B", "C"};
    m_table->setHorizontalHeaderLabels(headers);

    // 初始化默认数据
    for (int r = 0; r < m_table->rowCount(); ++r)
        for (int c = 0; c < m_table->columnCount(); ++c)
            m_table->setItem(r, c, new QTableWidgetItem(""));
}

void TableEditor::setTableData(const QVariantMap &data)
{
    // 断开信号避免触发 valueChanged
    m_table->blockSignals(true);

    m_table->clear();

    QStringList columns = data.value("columns").toStringList();
    QVariantList rows = data.value("rows").toList();

    int colCount = columns.size();
    int rowCount = rows.size();
    if (colCount == 0) colCount = 1;
    if (rowCount == 0) rowCount = 1;

    m_table->setColumnCount(colCount);
    m_table->setRowCount(rowCount);
    m_table->setHorizontalHeaderLabels(columns);

    for (int r = 0; r < rowCount; ++r) {
        QVariantList rowData;
        if (r < rows.size())
            rowData = rows[r].toList();
        for (int c = 0; c < colCount; ++c) {
            QString val;
            if (c < rowData.size())
                val = rowData[c].toString();
            m_table->setItem(r, c, new QTableWidgetItem(val));
        }
    }

    m_table->blockSignals(false);
}

QVariantMap TableEditor::tableData() const
{
    QVariantMap result;

    int colCount = m_table->columnCount();
    int rowCount = m_table->rowCount();

    // 列头
    QStringList columns;
    for (int c = 0; c < colCount; ++c) {
        QTableWidgetItem *hdr = m_table->horizontalHeaderItem(c);
        columns.append(hdr ? hdr->text() : QString("Col %1").arg(c + 1));
    }
    result["columns"] = columns;

    // 行数据
    QVariantList rows;
    for (int r = 0; r < rowCount; ++r) {
        QVariantList row;
        for (int c = 0; c < colCount; ++c) {
            QTableWidgetItem *item = m_table->item(r, c);
            row.append(item ? item->text() : "");
        }
        rows.append(QVariant(row));
    }
    result["rows"] = rows;

    return result;
}

void TableEditor::emitValueChanged()
{
    emit valueChanged();
}

// ---- Slots ----

void TableEditor::onCellChanged(int /*row*/, int /*col*/)
{
    emit valueChanged();
}

void TableEditor::onAddRow()
{
    int currentRow = m_table->currentRow();
    if (currentRow < 0) currentRow = m_table->rowCount() - 1;
    m_table->blockSignals(true);
    m_table->insertRow(currentRow + 1);
    for (int c = 0; c < m_table->columnCount(); ++c)
        m_table->setItem(currentRow + 1, c, new QTableWidgetItem(""));
    m_table->blockSignals(false);
    emit valueChanged();
}

void TableEditor::onRemoveRow()
{
    int currentRow = m_table->currentRow();
    if (currentRow >= 0 && m_table->rowCount() > 1) {
        m_table->blockSignals(true);
        m_table->removeRow(currentRow);
        m_table->blockSignals(false);
        emit valueChanged();
    }
}

void TableEditor::onAddColumn()
{
    int currentCol = m_table->currentColumn();
    if (currentCol < 0) currentCol = m_table->columnCount() - 1;
    m_table->blockSignals(true);
    m_table->insertColumn(currentCol + 1);
    QString header = QString("Col %1").arg(currentCol + 2);
    m_table->setHorizontalHeaderItem(currentCol + 1, new QTableWidgetItem(header));
    for (int r = 0; r < m_table->rowCount(); ++r)
        m_table->setItem(r, currentCol + 1, new QTableWidgetItem(""));
    m_table->blockSignals(false);
    emit valueChanged();
}

void TableEditor::onRemoveColumn()
{
    int currentCol = m_table->currentColumn();
    if (currentCol >= 0 && m_table->columnCount() > 1) {
        m_table->blockSignals(true);
        m_table->removeColumn(currentCol);
        m_table->blockSignals(false);
        emit valueChanged();
    }
}

void TableEditor::onPasteFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (text.isEmpty())
        return;

    // 按行拆分
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    if (lines.isEmpty())
        return;

    int startRow = m_table->currentRow();
    int startCol = m_table->currentColumn();
    if (startRow < 0) startRow = 0;
    if (startCol < 0) startCol = 0;

    m_table->blockSignals(true);

    for (int r = 0; r < lines.size(); ++r) {
        QStringList cols;
        // 尝试用制表符或逗号拆分
        if (lines[r].contains('\t'))
            cols = lines[r].split('\t');
        else if (lines[r].contains(','))
            cols = lines[r].split(',');
        else
            cols = QStringList{lines[r]};

        int targetRow = startRow + r;
        // 不够就新增行
        while (targetRow >= m_table->rowCount()) {
            m_table->insertRow(m_table->rowCount());
            for (int cc = 0; cc < m_table->columnCount(); ++cc)
                m_table->setItem(m_table->rowCount() - 1, cc, new QTableWidgetItem(""));
        }

        for (int c = 0; c < cols.size(); ++c) {
            int targetCol = startCol + c;
            if (targetCol < m_table->columnCount()) {
                QTableWidgetItem *item = m_table->item(targetRow, targetCol);
                if (item)
                    item->setText(cols[c].trimmed());
            }
        }
    }

    m_table->blockSignals(false);
    emit valueChanged();
}

// ---- 右键菜单 ----

void TableEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
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

    QAction *addRow    = menu.addAction("添加行");
    QAction *removeRow = menu.addAction("删除行");
    menu.addSeparator();
    QAction *addCol    = menu.addAction("添加列");
    QAction *removeCol = menu.addAction("删除列");
    menu.addSeparator();
    QAction *paste = menu.addAction("从剪贴板粘贴");

    QAction *chosen = menu.exec(event->globalPos());
    if (chosen == addRow)
        onAddRow();
    else if (chosen == removeRow)
        onRemoveRow();
    else if (chosen == addCol)
        onAddColumn();
    else if (chosen == removeCol)
        onRemoveColumn();
    else if (chosen == paste)
        onPasteFromClipboard();
}
