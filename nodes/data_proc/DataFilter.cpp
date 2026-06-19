// nodes/data_proc/DataFilter.cpp
#include "DataFilter.h"
#include "DataTable.h"

void DataFilter::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("column", "筛选列", ParamType::String, QString());
	addParam("operator", "运算符", ParamType::Combo, QString("大于"),
			 { {"options", QStringList{"等于", "不等于", "大于", "大于等于",
									  "小于", "小于等于", "包含", "不包含",
									  "为空", "不为空"}} });
	addParam("value", "比较值", ParamType::String, QString());
}

void DataFilter::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull())
	{
		reportError("输入数据为空");
		return;
	}

	auto table = inputData->toDataTable();
	if (!table)
	{
		reportError("无法获取DataTable");
		return;
	}

	QString colName = param("column").toString();
	if (!table->hasColumn(colName))
	{
		reportError(QString("列[%1]不存在").arg(colName));
		return;
	}

	QString op = param("operator").toString();
	QString valStr = param("value").toString();

	auto result = std::make_shared<DataTable>();
	QStringList colNames = table->columnNames();
	for (const auto& name : colNames)
	{
		result->addColumn(name);
	}

	int colIdx = table->columnIndex(colName);
	int rowCount = table->rowCount();

	for (int r = 0; r < rowCount; ++r)
	{
		QVariant cellVal = table->value(r, colIdx);
		bool match = false;

		if (op == "为空")
		{
			match = cellVal.isNull() || cellVal.toString().isEmpty();
		}
		else if (op == "不为空")
		{
			match = !cellVal.isNull() && !cellVal.toString().isEmpty();
		}
		else if (op == "包含")
		{
			match = cellVal.toString().contains(valStr);
		}
		else if (op == "不包含")
		{
			match = !cellVal.toString().contains(valStr);
		}
		else if (op == "等于")
		{
			bool okA, okB;
			double a = cellVal.toDouble(&okA);
			double b = valStr.toDouble(&okB);
			if (okA && okB)
				match = (a == b);
			else
				match = (cellVal.toString() == valStr);
		}
		else if (op == "不等于")
		{
			bool okA, okB;
			double a = cellVal.toDouble(&okA);
			double b = valStr.toDouble(&okB);
			if (okA && okB)
				match = (a != b);
			else
				match = (cellVal.toString() != valStr);
		}
		else
		{
			// 数值比较
			bool okA, okB;
			double a = cellVal.toDouble(&okA);
			double b = valStr.toDouble(&okB);
			if (!okA || !okB)
			{
				match = false;
			}
			else if (op == "大于") match = (a > b);
			else if (op == "大于等于") match = (a >= b);
			else if (op == "小于") match = (a < b);
			else if (op == "小于等于") match = (a <= b);
		}

		if (match)
		{
			result->addRow(table->row(r));
		}
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
