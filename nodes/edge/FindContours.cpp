// nodes/edge/FindContours.cpp
#include "FindContours.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <vector>

void FindContours::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
	addOutputPort("output_count", DataType::Numeric);

	addParam("mode", "检索模式", ParamType::Combo, QString("外轮廓(EXTERNAL)"),
			 { {"options", QStringList{"外轮廓(EXTERNAL)", "所有轮廓(LIST)", "树形(TREE)"}} });
	addParam("method", "近似方法", ParamType::Combo, QString("简单近似(SIMPLE)"),
			 { {"options", QStringList{"无近似(NONE)", "简单近似(SIMPLE)"}} });
	addParam("min_area", "最小面积", ParamType::Double, 0.0,
			 { {"min", 0.0}, {"max", 1000000.0} });
	addParam("draw_on_original", "在原图上绘制", ParamType::Bool, true);
	addParam("draw_color", "绘制颜色", ParamType::Color, QVariantList{ 0, 255, 0, 255 });
	addParam("line_width", "线宽", ParamType::IntSlider, 2, { {"min", 1}, {"max", 10} });
}

void FindContours::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	// 转灰度再二值化
	cv::Mat gray;
	if (src.channels() == 4)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
	}
	else if (src.channels() == 3)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	}
	else
	{
		gray = src.clone();
	}

	cv::Mat binary;
	cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

	// 检索模式
	QString modeName = param("mode").toString();
	int mode = cv::RETR_EXTERNAL;
	if (modeName.startsWith("所有")) mode = cv::RETR_LIST;
	else if (modeName.startsWith("树形")) mode = cv::RETR_TREE;

	// 近似方法
	QString methodName = param("method").toString();
	int method = cv::CHAIN_APPROX_SIMPLE;
	if (methodName.startsWith("无近似")) method = cv::CHAIN_APPROX_NONE;

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(binary, contours, mode, method);

	// 按最小面积过滤
	double minArea = param("min_area").toDouble();
	if (minArea > 0)
	{
		std::vector<std::vector<cv::Point>> filtered;
		for (const auto& c : contours)
		{
			if (cv::contourArea(c) >= minArea)
			{
				filtered.push_back(c);
			}
		}
		contours = filtered;
	}

	// 绘制
	QColor drawColor = Utility::arrayToColor(param("draw_color").toList());
	cv::Scalar color = Utility::colorToScalar(drawColor, false);
	int lineWidth = param("line_width").toInt();

	cv::Mat canvas;
	if (param("draw_on_original").toBool())
	{
		// 在原图上绘制，需要保证是三通道
		if (src.channels() == 1)
		{
			cv::cvtColor(src, canvas, cv::COLOR_GRAY2BGR);
		}
		else if (src.channels() == 4)
		{
			cv::cvtColor(src, canvas, cv::COLOR_BGRA2BGR);
		}
		else
		{
			canvas = src.clone();
		}
	}
	else
	{
		canvas = cv::Mat::zeros(src.size(), CV_8UC3);
	}

	cv::drawContours(canvas, contours, -1, color, lineWidth);

	setOutput("output_image", NodeData::createImage(canvas));
	setOutput("output_count", NodeData::createNumeric(static_cast<double>(contours.size())));
}
