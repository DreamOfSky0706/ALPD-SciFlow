// core/DataTable.h
#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVector>
#include <algorithm>

// 二维表格数据结构，按列存储
class DataTable
{
public:
	DataTable() = default;

	// 列操作
	int columnCount() const;
	int rowCount() const;
	QStringList columnNames() const;

	void addColumn(const QString& name, const QVariantList& data = {});
	void removeColumn(int index);
	void removeColumn(const QString& name);

	int columnIndex(const QString& name) const;
	bool hasColumn(const QString& name) const;
	QString columnName(int index) const;

	// 获取一整列的数据
	QVariantList columnData(int index) const;
	QVariantList columnData(const QString& name) const;

	// 行操作
	void addRow(const QVariantList& values);
	void removeRow(int index);
	QVariantList row(int index) const;

	// 单元格读写
	QVariant value(int row, int col) const;
	QVariant value(int row, const QString& colName) const;
	void setValue(int row, int col, const QVariant& val);

	// 创建一份深拷贝
	std::shared_ptr<DataTable> clone() const;

	// 清空全部数据
	void clear();

private:
	// 确保所有列长度对齐到最长列
	void alignColumns();

	QStringList m_columnNames;
	QVector<QVariantList> m_columns;
};
