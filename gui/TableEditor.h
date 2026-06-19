#pragma once

#include <QWidget>
#include <QVariantMap>
#include <QTableWidget>

// 嵌入式表格编辑器 —— 用于 DataTable 类节点
// 支持行列增删、剪贴板粘贴
class TableEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditor(QWidget *parent = nullptr);

    void setTableData(const QVariantMap &data);
    QVariantMap tableData() const;

signals:
    void valueChanged();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onCellChanged(int row, int col);
    void onAddRow();
    void onRemoveRow();
    void onAddColumn();
    void onRemoveColumn();
    void onPasteFromClipboard();

private:
    void setupTable();
    void emitValueChanged();

    QTableWidget *m_table = nullptr;
};
