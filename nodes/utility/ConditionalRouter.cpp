// nodes/utility/ConditionalRouter.cpp
#include "ConditionalRouter.h"

void ConditionalRouter::defineNode()
{
	addInputPort("input_data", DataType::Image);
	addInputPort("input_condition", DataType::Numeric);
	addOutputPort("output_true", DataType::Image);
	addOutputPort("output_false", DataType::Image);

	addParam("operator", "运算符", ParamType::Combo, QString("大于"),
			 { {"options", QStringList{"大于", "大于等于", "等于", "小于", "小于等于", "不等于"}} });
	addParam("threshold", "阈值", ParamType::Double, 0.0, { {"min", -1e9}, {"max", 1e9} });
}

void ConditionalRouter::process()
{
	auto imgData = getInput("input_data");
	auto condData = getInput("input_condition");
	if (!imgData || imgData->isNull())
	{
		reportError("输入数据为空"); return;
	}
	if (!condData || condData->isNull())
	{
		reportError("条件值为空"); return;
	}

	double val = condData->toNumeric();
	double thr = param("threshold").toDouble();
	QString op = param("operator").toString();

	bool result = false;
	if (op == "大于") result = (val > thr);
	else if (op == "大于等于") result = (val >= thr);
	else if (op == "等于") result = (std::abs(val - thr) < 1e-9);
	else if (op == "小于") result = (val < thr);
	else if (op == "小于等于") result = (val <= thr);
	else if (op == "不等于") result = (std::abs(val - thr) >= 1e-9);

	if (result)
	{
		setOutput("output_true", imgData);
		setOutput("output_false", std::make_shared<NodeData>());
	}
	else
	{
		setOutput("output_true", std::make_shared<NodeData>());
		setOutput("output_false", imgData);
	}
}
