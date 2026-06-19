// nodes/composite/OpacityAdjust.cpp
#include "OpacityAdjust.h"
#include <opencv2/imgproc.hpp>

void OpacityAdjust::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("opacity", "不透明度", ParamType::DoubleSlider, 1.0,
			 { {"min", 0.0}, {"max", 1.0} });
}

void OpacityAdjust::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	double opacity = param("opacity").toDouble();

	// 确保有Alpha通道
	cv::Mat withAlpha;
	if (src.channels() == 1)
	{
		cv::cvtColor(src, withAlpha, cv::COLOR_GRAY2BGRA);
	}
	else if (src.channels() == 3)
	{
		cv::cvtColor(src, withAlpha, cv::COLOR_BGR2BGRA);
	}
	else
	{
		withAlpha = src.clone();
	}

	// 修改Alpha通道
	std::vector<cv::Mat> channels;
	cv::split(withAlpha, channels);
	channels[3].convertTo(channels[3], CV_32F);
	channels[3] *= opacity;
	channels[3].convertTo(channels[3], CV_8U);
	cv::merge(channels, withAlpha);

	setOutput("output_image", NodeData::createImage(withAlpha));
}
