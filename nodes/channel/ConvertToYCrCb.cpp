// nodes/channel/ConvertToYCrCb.cpp
#include "ConvertToYCrCb.h"
#include <opencv2/imgproc.hpp>

void ConvertToYCrCb::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
}

void ConvertToYCrCb::process()
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
		reportError("转YCrCb需要彩色图像输入");
		return;
	}

	cv::Mat bgr;
	if (src.channels() == 4)
	{
		cv::cvtColor(src, bgr, cv::COLOR_BGRA2BGR);
		reportWarning("已自动移除Alpha通道");
	}
	else
	{
		bgr = src;
	}

	cv::Mat ycrcb;
	cv::cvtColor(bgr, ycrcb, cv::COLOR_BGR2YCrCb);
	setOutput("output_image", NodeData::createImage(ycrcb));
}
