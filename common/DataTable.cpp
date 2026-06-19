// core/DataTable.cpp
#include "DataTable.h"
#include <algorithm>

int DataTable::columnCount() const
{
	return m_columnNames.size();
}

int DataTable::rowCount() const
{
	int maxRows = 0;
	for (const auto& col : m_columns)
	{
		maxRows = std::max(maxRows, (int)col.size());
	}
	return maxRows;
}

QStringList DataTable::columnNames() const
{
	return m_columnNames;
}

void DataTable::addColumn(const QString& name, const QVariantList& data)
{
	m_columnNames.append(name);
	m_columns.append(data);
	alignColumns();
}

void DataTable::removeColumn(int index)
{
	if (index < 0 || index >= m_columnNames.size())
	{
		return;
	}
	m_columnNames.removeAt(index);
	m_columns.removeAt(index);
}

void DataTable::removeColumn(const QString& name)
{
	int idx = columnIndex(name);
	if (idx >= 0)
	{
		removeColumn(idx);
	}
}

int DataTable::columnIndex(const QString& name) const
{
	return m_columnNames.indexOf(name);
}

bool DataTable::hasColumn(const QString& name) const
{
	return m_columnNames.contains(name);
}

QString DataTable::columnName(int index) const
{
	if (index < 0 || index >= m_columnNames.size())
	{
		return QString();
	}
	return m_columnNames[index];
}

QVariantList DataTable::columnData(int index) const
{
	if (index < 0 || index >= m_columns.size())
	{
		return {};
	}
	return m_columns[index];
}

QVariantList DataTable::columnData(const QString& name) const
{
	int idx = columnIndex(name);
	return columnData(idx);
}

void DataTable::addRow(const QVariantList& values)
{
	// 如果还没有任何列，根据values的数量自动创建列
	if (m_columnNames.isEmpty())
	{
		for (int i = 0; i < values.size(); ++i)
		{
			m_columnNames.append(QString("Column_%1").arg(i));
			m_columns.append(QVariantList());
		}
	}

	int cols = m_columnNames.size();
	for (int i = 0; i < cols; ++i)
	{
		if (i < values.size())
		{
			m_columns[i].append(values[i]);
		}
		else
		{
			m_columns[i].append(QVariant());
		}
	}
}

void DataTable::removeRow(int index)
{
	if (index < 0 || index >= rowCount())
	{
		return;
	}
	for (auto& col : m_columns)
	{
		if (index < col.size())
		{
			col.removeAt(index);
		}
	}
}

QVariantList DataTable::row(int index) const
{
	QVariantList result;
	if (index < 0 || index >= rowCount())
	{
		return result;
	}
	for (const auto& col : m_columns)
	{
		if (index < col.size())
		{
			result.append(col[index]);
		}
		else
		{
			result.append(QVariant());
		}
	}
	return result;
}

QVariant DataTable::value(int row, int col) const
{
	if (col < 0 || col >= m_columns.size())
	{
		return QVariant();
	}
	if (row < 0 || row >= m_columns[col].size())
	{
		return QVariant();
	}
	return m_columns[col][row];
}

QVariant DataTable::value(int row, const QString& colName) const
{
	int idx = columnIndex(colName);
	return value(row, idx);
}

void DataTable::setValue(int row, int col, const QVariant& val)
{
	if (col < 0 || col >= m_columns.size())
	{
		return;
	}
	// 如果行不够长，用空值补齐
	while (m_columns[col].size() <= row)
	{
		m_columns[col].append(QVariant());
	}
	m_columns[col][row] = val;
}

std::shared_ptr<DataTable> DataTable::clone() const
{
	auto copy = std::make_shared<DataTable>();
	copy->m_columnNames = m_columnNames;
	copy->m_columns = m_columns;
	return copy;
}

void DataTable::clear()
{
	m_columnNames.clear();
	m_columns.clear();
}

void DataTable::alignColumns()
{
	int maxRows = rowCount();
	for (auto& col : m_columns)
	{
		while (col.size() < maxRows)
		{
			col.append(QVariant());
		}
	}
}
