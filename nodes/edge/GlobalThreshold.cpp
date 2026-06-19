// nodes/edge/GlobalThreshold.cpp
#include "GlobalThreshold.h"
#include <opencv2/imgproc.hpp>

void GlobalThreshold::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("threshold_value", "阈值", ParamType::IntSlider, 128,
			 { {"min", 0}, {"max", 255} });
	addParam("max_value", "最大值", ParamType::IntSlider, 255,
			 { {"min", 0}, {"max", 255} });
	addParam("type", "类型", ParamType::Combo, QString("二值化(BINARY)"),
			 { {"options", QStringList{"二值化(BINARY)", "反二值化(BINARY_INV)",
									  "截断(TRUNC)", "置零(TOZERO)", "反置零(TOZERO_INV)"}} });
}

void GlobalThreshold::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
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
		gray = src;
	}

	int threshVal = param("threshold_value").toInt();
	int maxVal = param("max_value").toInt();
	QString typeName = param("type").toString();

	int threshType = cv::THRESH_BINARY;
	if (typeName.startsWith("反二值化")) threshType = cv::THRESH_BINARY_INV;
	else if (typeName.startsWith("截断")) threshType = cv::THRESH_TRUNC;
	else if (typeName.startsWith("置零")) threshType = cv::THRESH_TOZERO;
	else if (typeName.startsWith("反置零")) threshType = cv::THRESH_TOZERO_INV;

	cv::Mat dst;
	cv::threshold(gray, dst, threshVal, maxVal, threshType);
	setOutput("output_image", NodeData::createImage(dst));
}
