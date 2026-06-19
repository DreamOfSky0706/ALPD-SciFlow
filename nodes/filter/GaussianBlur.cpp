// nodes/filter/GaussianBlur.cpp
#include "GaussianBlur.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void GaussianBlur::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("kernel_size", "核大小", ParamType::IntSlider, 5, { {"min", 1}, {"max", 99} });
	addParam("sigma", "Sigma", ParamType::DoubleSlider, 1.0, { {"min", 0.1}, {"max", 50.0} });
}

void GaussianBlur::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int ksize = Utility::ensureOdd(param("kernel_size").toInt());
	double sigma = param("sigma").toDouble();

	cv::Mat dst;
	cv::GaussianBlur(src, dst, cv::Size(ksize, ksize), sigma);
	setOutput("output_image", NodeData::createImage(dst));
}
