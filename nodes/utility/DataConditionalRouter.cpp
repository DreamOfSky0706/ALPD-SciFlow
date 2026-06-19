// nodes/utility/DataConditionalRouter.cpp
#include "DataConditionalRouter.h"
#include "DataTable.h"
#include <cmath>

void DataConditionalRouter::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addInputPort("input_condition", DataType::Numeric);
	addOutputPort("output_true", DataType::DataTable);
	addOutputPort("output_false", DataType::DataTable);

	addParam("operator", "运算符", ParamType::Combo, QString("大于"),
		{{"options",QStringList{"大于","大于等于","等于","小于","小于等于","不等于","不为零"}}});
	addParam("threshold", "阈值", ParamType::Double, 0.0);
}

void DataConditionalRouter::process()
{
	auto data = getInput("input_data");
	auto cond = getInput("input_condition");
	if (!data || data->isNull()) { reportError("输入数据为空"); return; }
	if (!cond || cond->isNull()) { reportError("条件值为空"); return; }

	double cv = cond->toNumeric();
	double th = param("threshold").toDouble();
	QString op = param("operator").toString();
	auto table = data->toDataTable();

	bool result = false;
	if (op == "大于") result = cv > th;
	else if (op == "大于等于") result = cv >= th;
	else if (op == "等于") result = std::abs(cv - th) < 1e-9;
	else if (op == "小于") result = cv < th;
	else if (op == "小于等于") result = cv <= th;
	else if (op == "不等于") result = std::abs(cv - th) >= 1e-9;
	else if (op == "不为零") result = std::abs(cv) >= 1e-9;

	if (result) {
		setOutput("output_true", NodeData::createDataTable(table));
		setOutput("output_false", NodeData::createDataTable(nullptr));
	} else {
		setOutput("output_true", NodeData::createDataTable(nullptr));
		setOutput("output_false", NodeData::createDataTable(table));
	}
}
