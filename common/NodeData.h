// core/NodeData.h
#pragma once

#include "DataType.h"
#include <memory>
#include <vector>
#include <opencv2/core.hpp>
#include <QString>

class DataTable;

// 节点间传递的数据载体，用类型标记+void指针实现类型擦除
class NodeData
{
public:
	NodeData();
	NodeData(DataType type, std::shared_ptr<void> data);

	bool isNull() const;
	DataType dataType() const;

	// 类型安全的数据获取，调用前需确认类型匹配
	cv::Mat toImage() const;
	std::vector<cv::Mat> toImageList() const;
	std::shared_ptr<DataTable> toDataTable() const;
	double toNumeric() const;
	QString toText() const;

	// 工厂方法
	static std::shared_ptr<NodeData> createImage(const cv::Mat& mat);
	static std::shared_ptr<NodeData> createImageList(const std::vector<cv::Mat>& list);
	static std::shared_ptr<NodeData> createDataTable(std::shared_ptr<DataTable> table);
	static std::shared_ptr<NodeData> createNumeric(double value);
	static std::shared_ptr<NodeData> createText(const QString& text);

private:
	DataType m_type;
	std::shared_ptr<void> m_data;
	bool m_null;
};
