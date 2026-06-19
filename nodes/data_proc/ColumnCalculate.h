// nodes/data_proc/ColumnCalculate.h
#pragma once

#include "NodeBase.h"

// 行/列运算：通过表达式创建新列或新行
class ColumnCalculate : public NodeBase
{
public:
	ColumnCalculate() = default;

	void defineNode() override;
	void process() override;

private:
	double evaluateSimple(const QString& expr, bool& ok) const;
	double evaluate(const QString& expr,
	                const QMap<QString, double>& vars,
	                bool& ok) const;
};
