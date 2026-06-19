// nodes/channel/ExtractAlpha.cpp
#include "ExtractAlpha.h"
#include <opencv2/imgproc.hpp>

void ExtractAlpha::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_alpha", DataType::Image);
}

void ExtractAlpha::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	if (src.channels() == 4)
	{
		std::vector<cv::Mat> channels;
		cv::split(src, channels);
		setOutput("output_alpha", NodeData::createImage(channels[3]));
	}
	else
	{
		// 无Alpha通道，输出全白蒙版
		cv::Mat white = cv::Mat(src.rows, src.cols, CV_8UC1, cv::Scalar(255));
		reportWarning("输入图像不含Alpha通道，已输出全白蒙版");
		setOutput("output_alpha", NodeData::createImage(white));
	}
}
