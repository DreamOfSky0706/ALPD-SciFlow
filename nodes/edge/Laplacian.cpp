// nodes/edge/Laplacian.cpp
#include "Laplacian.h"
#include <opencv2/imgproc.hpp>

void Laplacian::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("kernel_size", "核大小", ParamType::Combo, QString("3"),
			 { {"options", QStringList{"1", "3", "5", "7"}} });
}

void Laplacian::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	cv::Mat gray;

	if (src.channels() == 4)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGRA2GRAY);
	}
	else if (src.channels() == 3)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	}
	else
	{
		gray = src;
	}

	int ksize = param("kernel_size").toString().toInt();

	cv::Mat lap;
	cv::Laplacian(gray, lap, CV_32F, ksize);

	cv::Mat dst;
	cv::convertScaleAbs(lap, dst);
	setOutput("output_image", NodeData::createImage(dst));
}
