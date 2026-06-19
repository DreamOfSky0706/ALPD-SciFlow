// nodes/data_proc/ColumnCalculate.cpp
#include "ColumnCalculate.h"
#include "DataTable.h"
#include <QRegularExpression>
#include <cmath>
#include <algorithm>

void ColumnCalculate::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("direction", "计算方向", ParamType::Combo, QString("新列"),
			 { {"options", QStringList{"新列", "新行"}} });
	addParam("new_name", "新列/行名", ParamType::String, QString("new"));
	addParam("expression", "表达式", ParamType::String, QString(),
			 { {"placeholder", "新列: col_A/col_B*100  新行: col_A+col_B"} });
}

void ColumnCalculate::process()
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

	bool calcCol = param("direction").toString() == "新列";
	QString newName = param("new_name").toString();
	QString expression = param("expression").toString().trimmed();
	if (expression.isEmpty()) { reportError("表达式为空"); return; }

	auto result = table->clone();
	QStringList colNames = result->columnNames();
	int rowCount = result->rowCount();
	int colCount = result->columnCount();

	if (calcCol) {
		// 逐行计算，每行得一个值 → 新列
		QVariantList newCol;
		for (int r = 0; r < rowCount; ++r) {
			QMap<QString, double> vars;
			for (int c = 0; c < colCount; ++c) {
				bool ok; double v = result->value(r, c).toDouble(&ok);
				if (ok) vars[colNames[c]] = v;
			}
			QString expr = expression;
			QStringList sorted = vars.keys();
			std::sort(sorted.begin(), sorted.end(), [](const QString& a, const QString& b) { return a.length() > b.length(); });
			for (const auto& name : sorted)
				expr.replace(name, QString::number(vars[name], 'g', 15));
			bool ok;
			double val = evaluateSimple(expr, ok);
			newCol << (ok ? QVariant(val) : QVariant());
		}
		result->addColumn(newName, newCol);
	} else {
		// 逐列计算，每列得一个值 → 新行
		QVariantList newRow;
		for (int c = 0; c < colCount; ++c) {
			QMap<QString, double> vars;
			for (int r = 0; r < rowCount; ++r) {
				bool ok; double v = result->value(r, c).toDouble(&ok);
				if (ok) vars[QString("r%1").arg(r)] = v;
			}
			// Also add column reference
			bool ok; double colVal = result->value(0, c).toDouble(&ok);
			if (ok) vars["value"] = colVal;

			QString expr = expression;
			QStringList sorted = vars.keys();
			std::sort(sorted.begin(), sorted.end(), [](const QString& a, const QString& b) { return a.length() > b.length(); });
			for (const auto& name : sorted)
				expr.replace(name, QString::number(vars[name], 'g', 15));
			double val = evaluateSimple(expr, ok);
			newRow << (ok ? QVariant(val) : QVariant());
		}
		result->addRow(newRow);
	}

	setOutput("output_data", NodeData::createDataTable(result));
}

double ColumnCalculate::evaluateSimple(const QString& sanitized, bool& ok) const
{
	double result = sanitized.toDouble(&ok);
	if (ok) return result;

	static QRegularExpression simpleOp(R"(^(-?[\d.]+)\s*([+\-*/])\s*(-?[\d.]+)$)");
	auto match = simpleOp.match(sanitized.trimmed());
	if (match.hasMatch()) {
		double a = match.captured(1).toDouble();
		QString op = match.captured(2);
		double b = match.captured(3).toDouble();
		ok = true;
		if (op == "+") return a + b;
		if (op == "-") return a - b;
		if (op == "*") return a * b;
		if (op == "/") { if (std::abs(b) < 1e-15) { ok = false; return 0.0; } return a / b; }
	}
	ok = false;
	return 0.0;
}

double ColumnCalculate::evaluate(const QString& expr,
                                 const QMap<QString, double>& vars,
                                 bool& ok) const
{
	Q_UNUSED(vars)
	return evaluateSimple(expr, ok);
}
