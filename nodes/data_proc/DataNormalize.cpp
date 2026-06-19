// nodes/data_proc/DataNormalize.cpp
#include "DataNormalize.h"
#include "DataTable.h"
#include <cmath>
#include <algorithm>

void DataNormalize::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_data", DataType::DataTable);

	addParam("direction", "归一化方向", ParamType::Combo, QString("按列"),
			 { {"options", QStringList{"按列", "按行"}} });
	addParam("target_key", "目标", ParamType::String, QString());
	addParam("method", "方法", ParamType::Combo, QString("Min-Max归一化(映射到0-1)"),
			 { {"options", QStringList{"Min-Max归一化(映射到0-1)", "Z-Score标准化"}} });
}

void DataNormalize::process()
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

	bool byCol = param("direction").toString() == "按列";
	QString target = param("target_key").toString();
	QString method = param("method").toString();
	auto result = table->clone();

	if (byCol) {
		// 按列归一化：对指定列的值做归一化
		if (!table->hasColumn(target)) {
			reportError(QString("列[%1]不存在").arg(target));
			return;
		}
		int colIdx = table->columnIndex(target);
		QVector<double> nums; QVector<int> numRows;
		for (int r = 0; r < result->rowCount(); ++r) {
			bool ok; double v = result->value(r, colIdx).toDouble(&ok);
			if (ok) { nums.append(v); numRows.append(r); }
		}
		if (nums.isEmpty()) { reportWarning("目标列没有数值数据"); setOutput("output_data", NodeData::createDataTable(result)); return; }

		if (method.startsWith("Min-Max")) {
			double minV = *std::min_element(nums.begin(), nums.end());
			double maxV = *std::max_element(nums.begin(), nums.end());
			double range = maxV - minV;
			if (range < 1e-12) {
				for (int r : numRows) result->setValue(r, colIdx, 0.0);
			} else {
				for (int i = 0; i < nums.size(); ++i)
					result->setValue(numRows[i], colIdx, (nums[i]-minV)/range);
			}
		} else {
			double sum = 0; for (double v : nums) sum += v;
			double mean = sum / nums.size();
			double varSum = 0; for (double v : nums) varSum += (v-mean)*(v-mean);
			double stddev = std::sqrt(varSum / nums.size());
			if (stddev < 1e-12) {
				for (int r : numRows) result->setValue(r, colIdx, 0.0);
			} else {
				for (int i = 0; i < nums.size(); ++i)
					result->setValue(numRows[i], colIdx, (nums[i]-mean)/stddev);
			}
		}
	} else {
		// 按行归一化：对指定行的值做归一化
		bool ok; int rowIdx = target.toInt(&ok);
		if (!ok || rowIdx < 0 || rowIdx >= table->rowCount()) {
			reportError(QString("行索引[%1]无效").arg(target));
			return;
		}
		QVector<double> nums; QVector<int> numCols;
		for (int c = 0; c < result->columnCount(); ++c) {
			bool ok2; double v = result->value(rowIdx, c).toDouble(&ok2);
			if (ok2) { nums.append(v); numCols.append(c); }
		}
		if (nums.isEmpty()) { reportWarning("目标行没有数值数据"); setOutput("output_data", NodeData::createDataTable(result)); return; }

		if (method.startsWith("Min-Max")) {
			double minV = *std::min_element(nums.begin(), nums.end());
			double maxV = *std::max_element(nums.begin(), nums.end());
			double range = maxV - minV;
			if (range < 1e-12) {
				for (int c : numCols) result->setValue(rowIdx, c, 0.0);
			} else {
				for (int i = 0; i < nums.size(); ++i)
					result->setValue(rowIdx, numCols[i], (nums[i]-minV)/range);
			}
		} else {
			double sum = 0; for (double v : nums) sum += v;
			double mean = sum / nums.size();
			double varSum = 0; for (double v : nums) varSum += (v-mean)*(v-mean);
			double stddev = std::sqrt(varSum / nums.size());
			if (stddev < 1e-12) {
				for (int c : numCols) result->setValue(rowIdx, c, 0.0);
			} else {
				for (int i = 0; i < nums.size(); ++i)
					result->setValue(rowIdx, numCols[i], (nums[i]-mean)/stddev);
			}
		}
	}

	setOutput("output_data", NodeData::createDataTable(result));
}
