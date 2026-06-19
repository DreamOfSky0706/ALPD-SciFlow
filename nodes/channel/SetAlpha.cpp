// nodes/channel/SetAlpha.cpp
#include "SetAlpha.h"
#include <opencv2/imgproc.hpp>

void SetAlpha::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addInputPort("input_mask", DataType::Image);
	addOutputPort("output_image", DataType::Image);
}

void SetAlpha::process()
{
	auto imgData = getInput("input_image");
	auto maskData = getInput("input_mask");

	if (!imgData || imgData->isNull())
	{
		reportError("输入图像为空");
		return;
	}
	if (!maskData || maskData->isNull())
	{
		reportError("输入蒙版为空");
		return;
	}

	cv::Mat img = imgData->toImage();
	cv::Mat mask = maskData->toImage();

	// 蒙版须为单通道
	if (mask.channels() != 1)
	{
		cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
		reportWarning("蒙版非单通道，已自动转灰度");
	}

	// 尺寸不一致时缩放蒙版
	if (mask.size() != img.size())
	{
		cv::resize(mask, mask, img.size());
		reportWarning("蒙版尺寸与图像不一致，已自动缩放");
	}

	// 获取BGR通道
	cv::Mat bgr;
	if (img.channels() == 4)
	{
		cv::cvtColor(img, bgr, cv::COLOR_BGRA2BGR);
	}
	else if (img.channels() == 1)
	{
		cv::cvtColor(img, bgr, cv::COLOR_GRAY2BGR);
	}
	else
	{
		bgr = img;
	}

	// 合并为BGRA
	std::vector<cv::Mat> channels;
	cv::split(bgr, channels);
	channels.push_back(mask);

	cv::Mat result;
	cv::merge(channels, result);
	setOutput("output_image", NodeData::createImage(result));
}
