// nodes/data_proc/RemoveDuplicates.cpp
#include "RemoveDuplicates.h"
#include "DataTable.h"
#include <QSet>

void RemoveDuplicates::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);
	addOutputPort("output_removed_count", DataType::Numeric);

	addParam("keep", "保留策略", ParamType::Combo, QString("保留第一条"),
			 { {"options", QStringList{"保留第一条", "保留最后一条"}} });
}

void RemoveDuplicates::process()
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

	QString keep = param("keep").toString();

	// 以整行内容构建key判重
	QSet<QString> seen;
	QVector<int> keepRows;

	if (keep == "保留第一条")
	{
		for (int r = 0; r < table->rowCount(); ++r)
		{
			QVariantList row = table->row(r);
			QStringList keyParts;
			for (const auto& v : row)
			{
				keyParts << v.toString();
			}
			QString key = keyParts.join("|");

			if (!seen.contains(key))
			{
				seen.insert(key);
				keepRows.append(r);
			}
		}
	}
	else
	{
		// 保留最后一条：从后向前扫描
		for (int r = table->rowCount() - 1; r >= 0; --r)
		{
			QVariantList row = table->row(r);
			QStringList keyParts;
			for (const auto& v : row)
			{
				keyParts << v.toString();
			}
			QString key = keyParts.join("|");

			if (!seen.contains(key))
			{
				seen.insert(key);
				keepRows.prepend(r);
			}
		}
	}

	auto result = std::make_shared<DataTable>();
	for (const auto& name : table->columnNames())
	{
		result->addColumn(name);
	}
	for (int r : keepRows)
	{
		result->addRow(table->row(r));
	}

	int removedCount = table->rowCount() - result->rowCount();
	setOutput("output_data", NodeData::createDataTable(result));
	setOutput("output_removed_count", NodeData::createNumeric(static_cast<double>(removedCount)));
}
