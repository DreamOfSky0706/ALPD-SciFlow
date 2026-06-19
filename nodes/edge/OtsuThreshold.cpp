// nodes/edge/OtsuThreshold.cpp
#include "OtsuThreshold.h"
#include <opencv2/imgproc.hpp>

void OtsuThreshold::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
	addOutputPort("output_threshold", DataType::Numeric);

	addParam("max_value", "最大值", ParamType::IntSlider, 255, { {"min", 0}, {"max", 255} });
}

void OtsuThreshold::process()
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

	int maxVal = param("max_value").toInt();

	cv::Mat dst;
	double otsuThresh = cv::threshold(gray, dst, 0, maxVal,
									  cv::THRESH_BINARY | cv::THRESH_OTSU);

	setOutput("output_image", NodeData::createImage(dst));
	setOutput("output_threshold", NodeData::createNumeric(otsuThresh));
}
