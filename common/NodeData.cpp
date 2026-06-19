// core/NodeData.cpp
#include "NodeData.h"
#include "DataTable.h"

NodeData::NodeData()
	: m_type(DataType::Image)
	, m_null(true)
{
}

NodeData::NodeData(DataType type, std::shared_ptr<void> data)
	: m_type(type)
	, m_data(data)
	, m_null(!data)
{
}

bool NodeData::isNull() const
{
	return m_null;
}

DataType NodeData::dataType() const
{
	return m_type;
}

cv::Mat NodeData::toImage() const
{
	if (m_null || m_type != DataType::Image)
	{
		return cv::Mat();
	}
	auto ptr = std::static_pointer_cast<cv::Mat>(m_data);
	return *ptr;
}

std::vector<cv::Mat> NodeData::toImageList() const
{
	if (m_null || m_type != DataType::ImageList)
	{
		return {};
	}
	auto ptr = std::static_pointer_cast<std::vector<cv::Mat>>(m_data);
	return *ptr;
}

std::shared_ptr<DataTable> NodeData::toDataTable() const
{
	if (m_null || m_type != DataType::DataTable)
	{
		return nullptr;
	}
	return std::static_pointer_cast<DataTable>(m_data);
}

double NodeData::toNumeric() const
{
	if (m_null || m_type != DataType::Numeric)
	{
		return 0.0;
	}
	auto ptr = std::static_pointer_cast<double>(m_data);
	return *ptr;
}

QString NodeData::toText() const
{
	if (m_null || m_type != DataType::Text)
	{
		return QString();
	}
	auto ptr = std::static_pointer_cast<QString>(m_data);
	return *ptr;
}

std::shared_ptr<NodeData> NodeData::createImage(const cv::Mat& mat)
{
	auto data = std::make_shared<cv::Mat>(mat.clone());
	return std::make_shared<NodeData>(DataType::Image, data);
}

std::shared_ptr<NodeData> NodeData::createImageList(const std::vector<cv::Mat>& list)
{
	auto data = std::make_shared<std::vector<cv::Mat>>(list);
	return std::make_shared<NodeData>(DataType::ImageList, data);
}

std::shared_ptr<NodeData> NodeData::createDataTable(std::shared_ptr<DataTable> table)
{
	return std::make_shared<NodeData>(DataType::DataTable, table);
}

std::shared_ptr<NodeData> NodeData::createNumeric(double value)
{
	auto data = std::make_shared<double>(value);
	return std::make_shared<NodeData>(DataType::Numeric, data);
}

std::shared_ptr<NodeData> NodeData::createText(const QString& text)
{
	auto data = std::make_shared<QString>(text);
	return std::make_shared<NodeData>(DataType::Text, data);
}
