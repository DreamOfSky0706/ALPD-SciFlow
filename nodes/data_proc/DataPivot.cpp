// nodes/data_proc/DataPivot.cpp
#include "DataPivot.h"
#include "DataTable.h"
#include <QMap>

void DataPivot::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("row_field", "行字段", ParamType::String, QString());
	addParam("col_field", "列字段", ParamType::String, QString());
	addParam("value_field", "值字段", ParamType::String, QString());
	addParam("agg_func", "聚合函数", ParamType::Combo, QString("求和"),
			 {{"options",QStringList{"求和","均值","计数","最大值","最小值"}}});
}

void DataPivot::process()
{
	auto data = getInput("input_data");
	if (!data || data->isNull()) { reportError("输入数据为空"); return; }
	auto table = data->toDataTable();
	if (!table) { reportError("无法获取DataTable"); return; }

	QString rowF = param("row_field").toString();
	QString colF = param("col_field").toString();
	QString valF = param("value_field").toString();
	QString agg = param("agg_func").toString();

	if (!table->hasColumn(rowF) || !table->hasColumn(colF) || !table->hasColumn(valF)) {
		reportError("指定的字段不存在"); return;
	}

	// 收集行列组合 → 值列表
	QMap<QPair<QString,QString>, QVector<double>> groups;
	for (int r = 0; r < table->rowCount(); ++r) {
		QString rv = table->value(r, table->columnIndex(rowF)).toString();
		QString cv = table->value(r, table->columnIndex(colF)).toString();
		bool ok; double vv = table->value(r, table->columnIndex(valF)).toDouble(&ok);
		if (ok) groups[{rv, cv}].append(vv);
	}

	// 收集唯一行值和列值
	QSet<QString> rowVals, colVals;
	for (auto it = groups.begin(); it != groups.end(); ++it) {
		rowVals.insert(it.key().first);
		colVals.insert(it.key().second);
	}
	QStringList rows = rowVals.values(); rows.sort();
	QStringList cols = colVals.values(); cols.sort();

	// 构建透视表
	auto result = std::make_shared<DataTable>();
	result->addColumn(rowF);
	for (const auto& c : cols) result->addColumn(c);

	for (const auto& rv : rows) {
		QVariantList row; row << rv;
		for (const auto& cv : cols) {
			auto it = groups.find({rv, cv});
			if (it == groups.end()) { row << QVariant(); continue; }
			const auto& vals = it.value();
			double val = 0;
			if (agg == "求和") { for (double v:vals) val+=v; }
			else if (agg == "均值") { for (double v:vals) val+=v; val/=vals.size(); }
			else if (agg == "计数") val = (double)vals.size();
			else if (agg == "最大值") val = *std::max_element(vals.begin(),vals.end());
			else if (agg == "最小值") val = *std::min_element(vals.begin(),vals.end());
			row << val;
		}
		result->addRow(row);
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
