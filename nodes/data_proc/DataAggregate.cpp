// nodes/data_proc/DataAggregate.cpp
#include "DataAggregate.h"
#include "DataTable.h"
#include <algorithm>
#include <cmath>
#include <QMap>

void DataAggregate::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("group_column", "分组列(空=不分组)", ParamType::String, QString());
	addParam("agg_column", "聚合列", ParamType::String, QString());
	addParam("agg_function", "聚合函数", ParamType::Combo, QString("均值(Mean)"),
			 { {"options", QStringList{"求和(Sum)", "均值(Mean)", "中位数(Median)",
									  "最大值(Max)", "最小值(Min)", "计数(Count)",
									  "标准差(Std)"}} });
}

void DataAggregate::process()
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

	QString aggColName = param("agg_column").toString();
	if (!table->hasColumn(aggColName))
	{
		reportError(QString("聚合列[%1]不存在").arg(aggColName));
		return;
	}

	int aggIdx = table->columnIndex(aggColName);
	QString funcName = param("agg_function").toString();
	QString groupColName = param("group_column").toString().trimmed();

	// 聚合函数
	auto aggregate = [&](const QVector<double>& values) -> double
		{
			if (values.isEmpty()) return 0.0;

			if (funcName.startsWith("求和"))
			{
				double sum = 0;
				for (double v : values) sum += v;
				return sum;
			}
			if (funcName.startsWith("均值"))
			{
				double sum = 0;
				for (double v : values) sum += v;
				return sum / values.size();
			}
			if (funcName.startsWith("中位数"))
			{
				QVector<double> sorted = values;
				std::sort(sorted.begin(), sorted.end());
				int mid = sorted.size() / 2;
				if (sorted.size() % 2 == 0)
					return (sorted[mid - 1] + sorted[mid]) / 2.0;
				return sorted[mid];
			}
			if (funcName.startsWith("最大值"))
			{
				return *std::max_element(values.begin(), values.end());
			}
			if (funcName.startsWith("最小值"))
			{
				return *std::min_element(values.begin(), values.end());
			}
			if (funcName.startsWith("计数"))
			{
				return static_cast<double>(values.size());
			}
			if (funcName.startsWith("标准差"))
			{
				double sum = 0;
				for (double v : values) sum += v;
				double mean = sum / values.size();
				double varSum = 0;
				for (double v : values) varSum += (v - mean) * (v - mean);
				return std::sqrt(varSum / values.size());
			}
			return 0.0;
		};

	auto result = std::make_shared<DataTable>();

	if (groupColName.isEmpty() || !table->hasColumn(groupColName))
	{
		// 不分组，全表聚合
		QVector<double> values;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			bool ok;
			double v = table->value(r, aggIdx).toDouble(&ok);
			if (ok) values.append(v);
		}

		double aggResult = aggregate(values);
		result->addColumn(funcName.left(funcName.indexOf("(")));
		result->addRow({ aggResult });
	}
	else
	{
		// 按group_column分组
		int groupIdx = table->columnIndex(groupColName);
		QMap<QString, QVector<double>> groups;

		// 保持出现顺序
		QStringList groupOrder;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			QString groupKey = table->value(r, groupIdx).toString();
			if (!groups.contains(groupKey))
			{
				groupOrder.append(groupKey);
			}

			bool ok;
			double v = table->value(r, aggIdx).toDouble(&ok);
			if (ok)
			{
				groups[groupKey].append(v);
			}
		}

		result->addColumn(groupColName);
		result->addColumn(aggColName + "_" + funcName.left(funcName.indexOf("(")));

		for (const auto& key : groupOrder)
		{
			double aggResult = aggregate(groups[key]);
			result->addRow({ key, aggResult });
		}
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
