// nodes/filter/BilateralFilter.cpp
#include "BilateralFilter.h"
#include <opencv2/imgproc.hpp>

void BilateralFilter::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("diameter", "直径", ParamType::IntSlider, 9, { {"min", 1}, {"max", 50} });
	addParam("sigma_color", "颜色Sigma", ParamType::DoubleSlider, 75.0,
			 { {"min", 1.0}, {"max", 300.0} });
	addParam("sigma_space", "空间Sigma", ParamType::DoubleSlider, 75.0,
			 { {"min", 1.0}, {"max", 300.0} });
}

void BilateralFilter::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	// 双边滤波要求8位单通道或三通道
	// 四通道需分离Alpha
	cv::Mat toFilter, alpha;
	if (src.channels() == 4)
	{
		std::vector<cv::Mat> ch;
		cv::split(src, ch);
		alpha = ch[3].clone();
		cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, toFilter);
	}
	else
	{
		toFilter = src;
	}

	int d = param("diameter").toInt();
	double sigmaColor = param("sigma_color").toDouble();
	double sigmaSpace = param("sigma_space").toDouble();

	cv::Mat filtered;
	cv::bilateralFilter(toFilter, filtered, d, sigmaColor, sigmaSpace);

	if (!alpha.empty())
	{
		std::vector<cv::Mat> outCh;
		cv::split(filtered, outCh);
		outCh.push_back(alpha);
		cv::merge(outCh, filtered);
	}

	setOutput("output_image", NodeData::createImage(filtered));
}
