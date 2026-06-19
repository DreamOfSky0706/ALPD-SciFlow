// nodes/color/Invert.cpp
#include "Invert.h"
#include <opencv2/core.hpp>

void Invert::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
}

void Invert::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	cv::Mat dst;
	cv::bitwise_not(src, dst);
	setOutput("output_image", NodeData::createImage(dst));
}
