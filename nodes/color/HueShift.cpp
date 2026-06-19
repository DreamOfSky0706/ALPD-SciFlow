// nodes/color/HueShift.cpp
#include "HueShift.h"
#include <opencv2/imgproc.hpp>

void HueShift::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("hue_offset", "色相偏移", ParamType::IntSlider, 0, { {"min", -180}, {"max", 180} });
}

void HueShift::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	// 需要三通道BGR输入
	if (src.channels() == 1)
	{
		reportError("色相偏移需要彩色图像输入");
		return;
	}

	// 如果有Alpha通道需要分离
	cv::Mat bgr, alpha;
	if (src.channels() == 4)
	{
		std::vector<cv::Mat> channels;
		cv::split(src, channels);
		alpha = channels[3].clone();
		cv::merge(std::vector<cv::Mat>{channels[0], channels[1], channels[2]}, bgr);
	}
	else
	{
		bgr = src;
	}

	cv::Mat hsv;
	cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

	int offset = param("hue_offset").toInt();
	std::vector<cv::Mat> hsvChannels;
	cv::split(hsv, hsvChannels);

	// OpenCV中H范围为0-179
	for (int y = 0; y < hsvChannels[0].rows; ++y)
	{
		uchar* row = hsvChannels[0].ptr<uchar>(y);
		for (int x = 0; x < hsvChannels[0].cols; ++x)
		{
			int newH = (row[x] + offset) % 180;
			if (newH < 0) newH += 180;
			row[x] = static_cast<uchar>(newH);
		}
	}

	cv::merge(hsvChannels, hsv);
	cv::Mat result;
	cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);

	// 恢复Alpha
	if (!alpha.empty())
	{
		std::vector<cv::Mat> outChannels;
		cv::split(result, outChannels);
		outChannels.push_back(alpha);
		cv::merge(outChannels, result);
	}

	setOutput("output_image", NodeData::createImage(result));
}
