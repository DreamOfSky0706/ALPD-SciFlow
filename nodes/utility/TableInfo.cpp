// nodes/utility/TableInfo.cpp
#include "TableInfo.h"
#include "DataTable.h"

void TableInfo::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_rows", DataType::Numeric);
	addOutputPort("output_cols", DataType::Numeric);
	addOutputPort("output_empty_rate", DataType::Numeric);
	addOutputPort("output_numeric_cols", DataType::Numeric);
	addOutputPort("output_summary", DataType::Text);
	addOutputPort("output_columns", DataType::Text);
}

void TableInfo::process()
{
	auto d = getInput("input_data");
	if (!d || d->isNull()) { reportError("输入数据为空"); return; }
	auto t = d->toDataTable();
	if (!t) { reportError("无法获取DataTable"); return; }

	int rows = t->rowCount(), cols = t->columnCount();
	int totalCells = rows * cols;
	int emptyCells = 0, numericCols = 0;
	for (int c = 0; c < cols; ++c) {
		bool hasNum = false;
		for (int r = 0; r < rows; ++r) {
			QVariant v = t->value(r, c);
			if (v.isNull() || v.toString().isEmpty()) emptyCells++;
			else { bool ok; v.toDouble(&ok); if (ok) hasNum = true; }
		}
		if (hasNum) numericCols++;
	}
	double emptyRate = totalCells > 0 ? 100.0 * emptyCells / totalCells : 0;

	QString summary = QString("%1行×%2列, %3个数值列, 空值率%4%")
		.arg(rows).arg(cols).arg(numericCols).arg(QString::number(emptyRate,'f',1));
	QString colNames = t->columnNames().join(", ");

	setOutput("output_rows", NodeData::createNumeric(rows));
	setOutput("output_cols", NodeData::createNumeric(cols));
	setOutput("output_empty_rate", NodeData::createNumeric(emptyRate));
	setOutput("output_numeric_cols", NodeData::createNumeric(numericCols));
	setOutput("output_summary", NodeData::createText(summary));
	setOutput("output_columns", NodeData::createText(colNames));
}
