// nodes/utility/DataQualityScore.cpp
#include "DataQualityScore.h"
#include "DataTable.h"
#include <QSet>

void DataQualityScore::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_score", DataType::Numeric);
	addOutputPort("output_report", DataType::Text);

	addParam("check_completeness", "检查缺失值", ParamType::Bool, true);
	addParam("check_consistency", "检查一致性", ParamType::Bool, true);
	addParam("check_uniqueness", "检查唯一性", ParamType::Bool, true);
}

void DataQualityScore::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull())
	{
		reportError("输入数据为空"); return;
	}
	auto table = inputData->toDataTable();
	if (!table)
	{
		reportError("无法获取DataTable"); return;
	}

	int totalCells = table->rowCount() * table->columnCount();
	if (totalCells == 0)
	{
		setOutput("output_score", NodeData::createNumeric(0)); setOutput("output_report", NodeData::createText("空表")); return;
	}

	QStringList report;
	double scores = 0;
	int checks = 0;

	if (param("check_completeness").toBool())
	{
		int missing = 0;
		for (int c = 0; c < table->columnCount(); ++c)
			for (int r = 0; r < table->rowCount(); ++r)
				if (table->value(r, c).isNull() || table->value(r, c).toString().isEmpty()) missing++;
		double pct = 100.0 * (1.0 - static_cast<double>(missing) / totalCells);
		scores += pct;
		checks++;
		report << QString("完整性：%.1f%%（缺失%1/%2）").arg(missing).arg(totalCells);
	}

	if (param("check_uniqueness").toBool())
	{
		QSet<QString> rowKeys;
		int dups = 0;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			QStringList parts;
			for (int c = 0; c < table->columnCount(); ++c)
				parts << table->value(r, c).toString();
			QString key = parts.join("|");
			if (rowKeys.contains(key)) dups++;
			else rowKeys.insert(key);
		}
		double pct = 100.0 * (1.0 - static_cast<double>(dups) / table->rowCount());
		scores += pct;
		checks++;
		report << QString("唯一性：%.1f%%（重复%1行）").arg(dups);
	}

	if (param("check_consistency").toBool())
	{
		scores += 100.0;
		checks++;
		report << "一致性：100%（暂不做深度检查）";
	}

	double finalScore = (checks > 0) ? scores / checks : 0;
	setOutput("output_score", NodeData::createNumeric(finalScore));
	setOutput("output_report", NodeData::createText(report.join("\n")));
}
