// nodes/channel/ChannelMerge.cpp
#include "ChannelMerge.h"
#include <opencv2/imgproc.hpp>

void ChannelMerge::defineNode()
{
	addInputPort("input_ch0", DataType::Image);
	addInputPort("input_ch1", DataType::Image);
	addInputPort("input_ch2", DataType::Image);
	addInputPort("input_ch3", DataType::Image, false);
	addOutputPort("output_image", DataType::Image);

	addParam("color_space", "目标色彩空间", ParamType::Combo, QString("BGR"),
			 { {"options", QStringList{"BGR", "HSV", "Lab", "HLS"}} });
}

void ChannelMerge::process()
{
	auto d0 = getInput("input_ch0");
	auto d1 = getInput("input_ch1");
	auto d2 = getInput("input_ch2");
	auto d3 = getInput("input_ch3");

	if (!d0 || d0->isNull() || !d1 || d1->isNull() || !d2 || d2->isNull())
	{
		reportError("前三个通道输入不能为空");
		return;
	}

	cv::Mat ch0 = d0->toImage();
	cv::Mat ch1 = d1->toImage();
	cv::Mat ch2 = d2->toImage();

	// 验证均为单通道
	if (ch0.channels() != 1 || ch1.channels() != 1 || ch2.channels() != 1)
	{
		reportError("输入通道必须为单通道灰度图");
		return;
	}

	// 验证尺寸一致
	if (ch0.size() != ch1.size() || ch0.size() != ch2.size())
	{
		reportError("输入通道尺寸不一致");
		return;
	}

	std::vector<cv::Mat> channels = { ch0, ch1, ch2 };

	// 可选的Alpha通道
	bool hasAlpha = (d3 && !d3->isNull());
	cv::Mat alphaChannel;
	if (hasAlpha)
	{
		alphaChannel = d3->toImage();
		if (alphaChannel.channels() != 1)
		{
			reportWarning("Alpha通道非单通道，已自动转灰度");
			cv::cvtColor(alphaChannel, alphaChannel, cv::COLOR_BGR2GRAY);
		}
		if (alphaChannel.size() != ch0.size())
		{
			reportWarning("Alpha通道尺寸不一致，已自动缩放");
			cv::resize(alphaChannel, alphaChannel, ch0.size());
		}
	}

	cv::Mat merged;
	cv::merge(channels, merged);

	// 如果不是BGR空间，转回BGR
	QString space = param("color_space").toString();
	if (space == "HSV")
	{
		cv::cvtColor(merged, merged, cv::COLOR_HSV2BGR);
	}
	else if (space == "Lab")
	{
		cv::cvtColor(merged, merged, cv::COLOR_Lab2BGR);
	}
	else if (space == "HLS")
	{
		cv::cvtColor(merged, merged, cv::COLOR_HLS2BGR);
	}

	// 追加Alpha
	if (hasAlpha)
	{
		std::vector<cv::Mat> outCh;
		cv::split(merged, outCh);
		outCh.push_back(alphaChannel);
		cv::merge(outCh, merged);
	}

	setOutput("output_image", NodeData::createImage(merged));
}
