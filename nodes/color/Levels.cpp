// nodes/color/Levels.cpp
#include "Levels.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void Levels::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("input_black", "输入黑点", ParamType::IntSlider, 0, { {"min", 0}, {"max", 255} });
	addParam("input_white", "输入白点", ParamType::IntSlider, 255, { {"min", 0}, {"max", 255} });
	addParam("input_gamma", "Gamma", ParamType::DoubleSlider, 1.0, { {"min", 0.1}, {"max", 10.0} });
	addParam("output_black", "输出黑点", ParamType::IntSlider, 0, { {"min", 0}, {"max", 255} });
	addParam("output_white", "输出白点", ParamType::IntSlider, 255, { {"min", 0}, {"max", 255} });
}

void Levels::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int inBlack = param("input_black").toInt();
	int inWhite = param("input_white").toInt();
	double gamma = param("input_gamma").toDouble();
	int outBlack = param("output_black").toInt();
	int outWhite = param("output_white").toInt();

	if (inWhite <= inBlack)
	{
		reportError("输入白点必须大于输入黑点");
		return;
	}

	// 构建256项查找表
	cv::Mat lut(1, 256, CV_8UC1);
	uchar* lutData = lut.ptr<uchar>();

	double inRange = inWhite - inBlack;
	double outRange = outWhite - outBlack;

	for (int i = 0; i < 256; ++i)
	{
		// 归一化到输入范围
		double normalized;
		if (i <= inBlack)
		{
			normalized = 0.0;
		}
		else if (i >= inWhite)
		{
			normalized = 1.0;
		}
		else
		{
			normalized = (i - inBlack) / inRange;
		}

		// Gamma校正
		normalized = std::pow(normalized, 1.0 / gamma);

		// 映射到输出范围
		double output = outBlack + normalized * outRange;
		output = std::max(0.0, std::min(255.0, output));
		lutData[i] = static_cast<uchar>(safeRound((output)));
	}

	cv::Mat dst;
	cv::LUT(src, lut, dst);
	setOutput("output_image", NodeData::createImage(dst));
}
