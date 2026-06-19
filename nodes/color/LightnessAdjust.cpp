// nodes/color/LightnessAdjust.cpp
#include "LightnessAdjust.h"
#include <opencv2/imgproc.hpp>

void LightnessAdjust::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("lightness_offset", "明度偏移", ParamType::IntSlider, 0,
			 { {"min", -100}, {"max", 100} });
}

void LightnessAdjust::process()
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
		reportError("明度调整需要彩色图像输入");
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

	int offset = param("lightness_offset").toInt();

	cv::Mat hls;
	cv::cvtColor(bgr, hls, cv::COLOR_BGR2HLS);

	std::vector<cv::Mat> hlsCh;
	cv::split(hls, hlsCh);

	// L通道加偏移
	hlsCh[1].convertTo(hlsCh[1], CV_32F);
	hlsCh[1] += offset;
	cv::threshold(hlsCh[1], hlsCh[1], 255.0, 255.0, cv::THRESH_TRUNC);
	cv::threshold(hlsCh[1], hlsCh[1], 0.0, 0.0, cv::THRESH_TOZERO);
	hlsCh[1].convertTo(hlsCh[1], CV_8U);

	cv::merge(hlsCh, hls);
	cv::Mat result;
	cv::cvtColor(hls, result, cv::COLOR_HLS2BGR);

	if (!alpha.empty())
	{
		std::vector<cv::Mat> outCh;
		cv::split(result, outCh);
		outCh.push_back(alpha);
		cv::merge(outCh, result);
	}

	setOutput("output_image", NodeData::createImage(result));
}
