// nodes/data_proc/DataProfile.cpp
#include "DataProfile.h"
#include "DataTable.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

void DataProfile::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);
	addOutputPort("output_report", DataType::Text);
}

void DataProfile::process()
{
	auto data = getInput("input_data");
	if (!data || data->isNull()) { reportError("输入数据为空"); return; }
	auto table = data->toDataTable();
	if (!table) { reportError("无法获取DataTable"); return; }

	auto profile = std::make_shared<DataTable>();
	QStringList cols = {"列名","类型","非空数","空值数","空值率%","最小值","最大值","均值","标准差","中位数","唯一值数"};
	for (const auto& c : cols) profile->addColumn(c);

	QStringList reportLines;
	reportLines << QString("=== 数据画像报告 ===");
	reportLines << QString("总行数: %1  总列数: %2").arg(table->rowCount()).arg(table->columnCount());

	for (int c = 0; c < table->columnCount(); ++c) {
		QString name = table->columnName(c);
		QVector<double> nums;
		QSet<QString> uniq;
		int nullCount = 0;
		for (int r = 0; r < table->rowCount(); ++r) {
			QVariant v = table->value(r, c);
			uniq.insert(v.toString());
			if (v.isNull() || v.toString().isEmpty()) { nullCount++; continue; }
			bool ok; double n = v.toDouble(&ok);
			if (ok) nums.append(n);
		}
		int nonNull = table->rowCount() - nullCount;
		double nullRate = 100.0 * nullCount / table->rowCount();
		QString type = nums.size() > nonNull/2 ? "数值" : "文本";

		QVariantList row;
		row << name << type << nonNull << nullCount << QString::number(nullRate,'f',1);
		if (!nums.isEmpty()) {
			double mn = *std::min_element(nums.begin(),nums.end());
			double mx = *std::max_element(nums.begin(),nums.end());
			double sum=0; for(double v:nums) sum+=v;
			double mean=sum/nums.size();
			double var=0; for(double v:nums) var+=(v-mean)*(v-mean);
			double sd=std::sqrt(var/nums.size());
			std::sort(nums.begin(),nums.end());
			double med=nums[nums.size()/2];
			if(nums.size()%2==0) med=(nums[nums.size()/2-1]+med)/2.0;
			row << mn << mx << QString::number(mean,'g',4) << QString::number(sd,'g',4) << QString::number(med,'g',4);
		} else { row << QVariant() << QVariant() << QVariant() << QVariant() << QVariant(); }
		row << uniq.size();
		profile->addRow(row);

		reportLines << QString("  [%1] %2  非空:%3(%4%%)  唯一值:%5")
			.arg(type, name).arg(nonNull).arg(QString::number(nullRate,'f',1)).arg(uniq.size());
	}

	reportLines << "=== 报告结束 ===";
	Logger::instance().success("数据画像完成");
	setOutput("output_data", NodeData::createDataTable(profile));
	setOutput("output_report", NodeData::createText(reportLines.join("\n")));
}
