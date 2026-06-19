// nodes/morphology/MorphBase.cpp
#include "MorphBase.h"
#include <opencv2/imgproc.hpp>

void MorphBase::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("kernel_shape", "核形状", ParamType::Combo, QString("矩形"),
			 { {"options", QStringList{"矩形", "椭圆", "十字"}} });
	addParam("kernel_size", "核大小", ParamType::IntSlider, 3, { {"min", 1}, {"max", 50} });
	addParam("iterations", "迭代次数", ParamType::IntSlider, 1, { {"min", 1}, {"max", 20} });
}

void MorphBase::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	QString shapeName = param("kernel_shape").toString();
	int shape = cv::MORPH_RECT;
	if (shapeName == "椭圆") shape = cv::MORPH_ELLIPSE;
	else if (shapeName == "十字") shape = cv::MORPH_CROSS;

	int ksize = param("kernel_size").toInt();
	if (ksize < 1) ksize = 1;
	int iterations = param("iterations").toInt();
	if (iterations < 1) iterations = 1;

	cv::Mat element = cv::getStructuringElement(shape, cv::Size(ksize, ksize));

	int op = morphOperation();
	cv::Mat dst;
	cv::morphologyEx(src, dst, op, element, cv::Point(-1, -1), iterations);

	setOutput("output_image", NodeData::createImage(dst));
}
