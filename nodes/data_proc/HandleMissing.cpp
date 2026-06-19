// nodes/data_proc/HandleMissing.cpp
#include "HandleMissing.h"
#include "DataTable.h"
#include <algorithm>

void HandleMissing::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("method", "处理方式", ParamType::Combo, QString("删除含缺失值的行"),
			 { {"options", QStringList{
				"删除含缺失值的行", "填充固定值",
				"填充列均值", "填充列中位数",
				"前向填充（用上一行的值）",
				"删除含缺失值的列",
				"填充行均值", "填充行中位数"}} });
	addParam("fill_value", "填充值", ParamType::String, QString("0"),
			 { {"visible_when", "method==填充固定值"} });
}

void HandleMissing::process()
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

	auto result = table->clone();
	QString method = param("method").toString();

	// ---- 按行操作 ----
	if (method == "删除含缺失值的行") {
		for (int r = result->rowCount() - 1; r >= 0; --r) {
			bool hasMissing = false;
			for (int c = 0; c < result->columnCount(); ++c) {
				QVariant v = result->value(r, c);
				if (v.isNull() || v.toString().isEmpty()) { hasMissing = true; break; }
			}
			if (hasMissing) result->removeRow(r);
		}
	}
	else if (method == "填充固定值") {
		QString fillVal = param("fill_value").toString();
		bool isNum; double numVal = fillVal.toDouble(&isNum);
		for (int c = 0; c < result->columnCount(); ++c)
			for (int r = 0; r < result->rowCount(); ++r) {
				QVariant v = result->value(r, c);
				if (v.isNull() || v.toString().isEmpty())
					result->setValue(r, c, isNum ? QVariant(numVal) : QVariant(fillVal));
			}
	}
	else if (method == "填充列均值" || method == "填充列中位数") {
		for (int c = 0; c < result->columnCount(); ++c) {
			QVector<double> nums;
			for (int r = 0; r < result->rowCount(); ++r) {
				QVariant v = result->value(r, c);
				bool ok; double num = v.toDouble(&ok);
				if (ok && !v.isNull()) nums.append(num);
			}
			if (nums.isEmpty()) continue;
			double fillVal;
			if (method == "填充列均值") {
				double sum = 0; for (double n : nums) sum += n;
				fillVal = sum / nums.size();
			} else {
				std::sort(nums.begin(), nums.end());
				int mid = nums.size() / 2;
				fillVal = (nums.size() % 2 == 0) ? (nums[mid-1]+nums[mid])/2.0 : nums[mid];
			}
			for (int r = 0; r < result->rowCount(); ++r) {
				QVariant v = result->value(r, c);
				if (v.isNull() || v.toString().isEmpty()) result->setValue(r, c, fillVal);
			}
		}
	}
	else if (method.startsWith("前向填充")) {
		for (int c = 0; c < result->columnCount(); ++c) {
			QVariant lastValid;
			for (int r = 0; r < result->rowCount(); ++r) {
				QVariant v = result->value(r, c);
				if (v.isNull() || v.toString().isEmpty()) {
					if (!lastValid.isNull()) result->setValue(r, c, lastValid);
				} else lastValid = v;
			}
		}
	}
	// ---- 按列操作 ----
	else if (method == "删除含缺失值的列") {
		for (int c = result->columnCount() - 1; c >= 0; --c) {
			bool hasMissing = false;
			for (int r = 0; r < result->rowCount(); ++r) {
				QVariant v = result->value(r, c);
				if (v.isNull() || v.toString().isEmpty()) { hasMissing = true; break; }
			}
			if (hasMissing) result->removeColumn(c);
		}
	}
	else if (method == "填充行均值" || method == "填充行中位数") {
		for (int r = 0; r < result->rowCount(); ++r) {
			QVector<double> nums;
			QVector<int> missingCols;
			for (int c = 0; c < result->columnCount(); ++c) {
				QVariant v = result->value(r, c);
				bool ok; double num = v.toDouble(&ok);
				if (ok && !v.isNull()) nums.append(num);
				else if (v.isNull() || v.toString().isEmpty()) missingCols.append(c);
			}
			if (nums.isEmpty() || missingCols.isEmpty()) continue;
			double fillVal;
			if (method == "填充行均值") {
				double sum = 0; for (double n : nums) sum += n;
				fillVal = sum / nums.size();
			} else {
				std::sort(nums.begin(), nums.end());
				int mid = nums.size() / 2;
				fillVal = (nums.size() % 2 == 0) ? (nums[mid-1]+nums[mid])/2.0 : nums[mid];
			}
			for (int c : missingCols) result->setValue(r, c, fillVal);
		}
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
