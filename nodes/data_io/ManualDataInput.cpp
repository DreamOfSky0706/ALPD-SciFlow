// nodes/data_io/ManualDataInput.cpp
#include "ManualDataInput.h"
#include "DataTable.h"

void ManualDataInput::defineNode()
{
	addOutputPort("output_data", DataType::DataTable);

	// 表格数据通过TableEditor控件编辑
	// 序列化格式: {"columns":["A","B"],"rows":[[1,"x"],[2,"y"]]}
	QVariantMap defaultData;
	defaultData["columns"] = QVariantList{ QString("Column_0"), QString("Column_1") };
	defaultData["rows"] = QVariantList();
	addParam("table_data", "表格数据", ParamType::TableEditor, defaultData);
}

void ManualDataInput::process()
{
	QVariantMap tableData = param("table_data").toMap();
	QVariantList columns = tableData.value("columns").toList();
	QVariantList rows = tableData.value("rows").toList();

	auto table = std::make_shared<DataTable>();

	for (const auto& col : columns)
	{
		table->addColumn(col.toString());
	}

	for (const auto& row : rows)
	{
		QVariantList values = row.toList();
		QVariantList converted;
		for (const auto& v : values)
		{
			bool ok;
			double num = v.toDouble(&ok);
			if (ok && v.type() != QVariant::String)
			{
				converted << num;
			}
			else
			{
				converted << v;
			}
		}
		table->addRow(converted);
	}

	setOutput("output_data", NodeData::createDataTable(table));
}
