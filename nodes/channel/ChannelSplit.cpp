// nodes/channel/ChannelSplit.cpp
#include "ChannelSplit.h"
#include <opencv2/imgproc.hpp>

void ChannelSplit::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_ch0", DataType::Image);
	addOutputPort("output_ch1", DataType::Image);
	addOutputPort("output_ch2", DataType::Image);
	addOutputPort("output_ch3", DataType::Image);

	addParam("color_space", "色彩空间", ParamType::Combo,
			 QString("BGR（拆为B/G/R）"),
			 { {"options", QStringList{"BGR（拆为B/G/R）", "HSV（拆为H/S/V）",
									  "Lab（拆为L/a/b）", "HLS（拆为H/L/S）"}} });
}

void ChannelSplit::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	if (src.channels() < 3)
	{
		// 单通道直接输出到ch0
		setOutput("output_ch0", NodeData::createImage(src));
		setOutput("output_ch1", std::make_shared<NodeData>());
		setOutput("output_ch2", std::make_shared<NodeData>());
		setOutput("output_ch3", std::make_shared<NodeData>());
		return;
	}

	// 分离Alpha（如果有）
	cv::Mat bgr, alpha;
	if (src.channels() == 4)
	{
		std::vector<cv::Mat> allCh;
		cv::split(src, allCh);
		alpha = allCh[3].clone();
		cv::merge(std::vector<cv::Mat>{allCh[0], allCh[1], allCh[2]}, bgr);
	}
	else
	{
		bgr = src;
	}

	// 色彩空间转换
	QString space = param("color_space").toString();
	cv::Mat converted;

	if (space.startsWith("HSV"))
	{
		cv::cvtColor(bgr, converted, cv::COLOR_BGR2HSV);
	}
	else if (space.startsWith("Lab"))
	{
		cv::cvtColor(bgr, converted, cv::COLOR_BGR2Lab);
	}
	else if (space.startsWith("HLS"))
	{
		cv::cvtColor(bgr, converted, cv::COLOR_BGR2HLS);
	}
	else
	{
		converted = bgr;
	}

	std::vector<cv::Mat> channels;
	cv::split(converted, channels);

	setOutput("output_ch0", NodeData::createImage(channels[0]));
	setOutput("output_ch1", NodeData::createImage(channels[1]));
	setOutput("output_ch2", NodeData::createImage(channels[2]));

	if (!alpha.empty())
	{
		setOutput("output_ch3", NodeData::createImage(alpha));
	}
	else
	{
		setOutput("output_ch3", std::make_shared<NodeData>());
	}
}
