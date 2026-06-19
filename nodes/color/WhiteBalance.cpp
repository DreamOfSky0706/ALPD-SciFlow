// nodes/color/WhiteBalance.cpp
#include "WhiteBalance.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void WhiteBalance::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("temperature", "色温(K)", ParamType::IntSlider, 6500,
			 { {"min", 2000}, {"max", 10000} });
}

void WhiteBalance::process()
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
		reportError("白平衡调整需要彩色图像输入");
		return;
	}

	int temp = param("temperature").toInt();

	// 以6500K为中性点，偏离越多调整越大
	// 低色温偏暖：增R减B，高色温偏冷：增B减R
	double deviation = (temp - 6500.0) / 4500.0; // 归一化到约[-1, +0.78]
	double rGain = 1.0 - deviation * 0.15;
	double bGain = 1.0 + deviation * 0.15;

	// clamp增益范围
	rGain = std::clamp(rGain, 0.5, 1.5);
	bGain = std::clamp(bGain, 0.5, 1.5);

	cv::Mat dst = src.clone();
	int channels = dst.channels();

	for (int y = 0; y < dst.rows; ++y)
	{
		uchar* row = dst.ptr<uchar>(y);
		for (int x = 0; x < dst.cols; ++x)
		{
			int idx = x * channels;
			// BGR顺序
			int b = safeRound((row[idx] * bGain));
			int r = safeRound((row[idx + 2] * rGain));
			row[idx] = static_cast<uchar>(std::clamp(b, 0, 255));
			row[idx + 2] = static_cast<uchar>(std::clamp(r, 0, 255));
		}
	}

	setOutput("output_image", NodeData::createImage(dst));
}
