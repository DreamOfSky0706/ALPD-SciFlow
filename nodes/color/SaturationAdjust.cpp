// nodes/color/SaturationAdjust.cpp
#include "SaturationAdjust.h"
#include <opencv2/imgproc.hpp>

void SaturationAdjust::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("saturation_factor", "饱和度倍率", ParamType::DoubleSlider, 1.0,
			 { {"min", 0.0}, {"max", 3.0} });
}

void SaturationAdjust::process()
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
		reportError("饱和度调整需要彩色图像输入");
		return;
	}

	cv::Mat bgr, alpha;
	if (src.channels() == 4)
	{
		std::vector<cv::Mat> ch;
		cv::split(src, ch);
		alpha = ch[3].clone();
		cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, bgr);
	}
	else
	{
		bgr = src;
	}

	double factor = param("saturation_factor").toDouble();

	cv::Mat hsv;
	cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);

	std::vector<cv::Mat> hsvCh;
	cv::split(hsv, hsvCh);

	// S通道乘以factor
	hsvCh[1].convertTo(hsvCh[1], CV_32F);
	hsvCh[1] *= factor;

	// clamp到0-255
	cv::threshold(hsvCh[1], hsvCh[1], 255.0, 255.0, cv::THRESH_TRUNC);
	cv::threshold(hsvCh[1], hsvCh[1], 0.0, 0.0, cv::THRESH_TOZERO);
	hsvCh[1].convertTo(hsvCh[1], CV_8U);

	cv::merge(hsvCh, hsv);
	cv::Mat result;
	cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);

	if (!alpha.empty())
	{
		std::vector<cv::Mat> outCh;
		cv::split(result, outCh);
		outCh.push_back(alpha);
		cv::merge(outCh, result);
	}

	setOutput("output_image", NodeData::createImage(result));
}
