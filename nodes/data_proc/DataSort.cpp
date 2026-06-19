// nodes/data_proc/DataSort.cpp
#include "DataSort.h"
#include "DataTable.h"
#include <algorithm>

void DataSort::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("direction", "排序对象", ParamType::Combo, QString("按行"),
			 { {"options", QStringList{"按行（排序列）", "按列（排序行）"}} });
	addParam("sort_key", "排序键", ParamType::String, QString());
	addParam("order", "排序方向", ParamType::Combo, QString("升序"),
			 { {"options", QStringList{"升序", "降序"}} });
}

void DataSort::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull()) {
		reportError("输入数据为空");
		return;
	}

	auto table = inputData->toDataTable();
	if (!table) {
		reportError("无法获取DataTable");
		return;
	}

	bool sortRows = param("direction").toString().startsWith("按行");
	QString key = param("sort_key").toString();
	bool ascending = (param("order").toString() == "升序");
	auto result = std::make_shared<DataTable>();

	if (sortRows) {
		// 按指定列的值排序行
		if (!table->hasColumn(key)) {
			reportError(QString("列[%1]不存在").arg(key));
			return;
		}
		int colIdx = table->columnIndex(key);
		QVector<int> indices(table->rowCount());
		for (int i = 0; i < indices.size(); ++i) indices[i] = i;
		std::sort(indices.begin(), indices.end(), [&](int a, int b) {
			QVariant va = table->value(a, colIdx), vb = table->value(b, colIdx);
			bool okA, okB;
			double na = va.toDouble(&okA), nb = vb.toDouble(&okB);
			int cmp;
			if (okA && okB) cmp = (na < nb) ? -1 : (na > nb) ? 1 : 0;
			else cmp = va.toString().compare(vb.toString());
			return ascending ? (cmp < 0) : (cmp > 0);
		});
		for (const auto& name : table->columnNames()) result->addColumn(name);
		for (int idx : indices) result->addRow(table->row(idx));
	} else {
		// 按指定行的值排序列
		bool ok; int rowIdx = key.toInt(&ok);
		if (!ok || rowIdx < 0 || rowIdx >= table->rowCount()) {
			reportError(QString("行索引[%1]无效（应为0~%2的数字）").arg(key).arg(table->rowCount()-1));
			return;
		}
		QVector<int> colIndices(table->columnCount());
		for (int i = 0; i < colIndices.size(); ++i) colIndices[i] = i;
		std::sort(colIndices.begin(), colIndices.end(), [&](int a, int b) {
			QVariant va = table->value(rowIdx, a), vb = table->value(rowIdx, b);
			bool okA, okB;
			double na = va.toDouble(&okA), nb = vb.toDouble(&okB);
			int cmp;
			if (okA && okB) cmp = (na < nb) ? -1 : (na > nb) ? 1 : 0;
			else cmp = va.toString().compare(vb.toString());
			return ascending ? (cmp < 0) : (cmp > 0);
		});
		// 按新顺序重建列
		for (int ci : colIndices) result->addColumn(table->columnName(ci));
		for (int r = 0; r < table->rowCount(); ++r) {
			QVariantList rowVals;
			for (int ci : colIndices) rowVals << table->value(r, ci);
			result->addRow(rowVals);
		}
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
