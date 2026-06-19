// core/DataType.h
#pragma once

#include <QString>
#include <QSet>

// 端口数据类型枚举
enum class DataType
{
	Image,      // 单张图像 cv::Mat
	ImageList,  // 图像列表 vector<shared_ptr<cv::Mat>>
	DataTable,  // 表格数据
	Numeric,    // 单个数值 double
	Text        // 文本字符串 QString
};

// 数据类型的显示名称
inline QString dataTypeName(DataType type)
{
	switch (type)
	{
	case DataType::Image:
		return "Image";
	case DataType::ImageList:
		return "ImageList";
	case DataType::DataTable:
		return "DataTable";
	case DataType::Numeric:
		return "Numeric";
	case DataType::Text:
		return "Text";
	}
	return "Unknown";
}

// 数据类型的短缩写，用于端口旁标注
inline QString dataTypeAbbrev(DataType type)
{
	switch (type)
	{
	case DataType::Image:
		return "Img";
	case DataType::ImageList:
		return "ImgL";
	case DataType::DataTable:
		return "DT";
	case DataType::Numeric:
		return "Num";
	case DataType::Text:
		return "Txt";
	}
	return "?";
}

// 数据类型对应的端口颜色(十六进制)
inline QString dataTypeColor(DataType type)
{
	switch (type)
	{
	case DataType::Image:
		return "#4EC9B0";
	case DataType::ImageList:
		return "#2D8B7A";
	case DataType::DataTable:
		return "#E0944A";
	case DataType::Numeric:
		return "#6BCB77";
	case DataType::Text:
		return "#C882D9";
	}
	return "#888888";
}
