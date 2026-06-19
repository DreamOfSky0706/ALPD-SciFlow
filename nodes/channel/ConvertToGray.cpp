// nodes/channel/ConvertToGray.cpp
#include "ConvertToGray.h"
#include <opencv2/imgproc.hpp>

void ConvertToGray::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
}

void ConvertToGray::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	if (src.channels() == 1)
	{
		setOutput("output_image", NodeData::createImage(src));
		return;
	}

	cv::Mat gray;
	if (src.channels() == 4)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
	}
	else
	{
		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	}

	setOutput("output_image", NodeData::createImage(gray));
}
