// nodes/edge/Sobel.cpp
#include "Sobel.h"
#include <opencv2/imgproc.hpp>

void Sobel::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("direction", "方向", ParamType::Combo, QString("X和Y合并"),
			 { {"options", QStringList{"水平(X)", "垂直(Y)", "X和Y合并"}} });
	addParam("kernel_size", "核大小", ParamType::Combo, QString("3"),
			 { {"options", QStringList{"3", "5", "7"}} });
}

void Sobel::process()
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

	QString dir = param("direction").toString();
	int ksize = param("kernel_size").toString().toInt();

	cv::Mat dst;

	if (dir == "X和Y合并")
	{
		cv::Mat gradX, gradY;
		cv::Sobel(gray, gradX, CV_32F, 1, 0, ksize);
		cv::Sobel(gray, gradY, CV_32F, 0, 1, ksize);
		cv::Mat mag;
		cv::magnitude(gradX, gradY, mag);
		cv::normalize(mag, dst, 0, 255, cv::NORM_MINMAX, CV_8U);
	}
	else if (dir == "水平(X)")
	{
		cv::Mat gradX;
		cv::Sobel(gray, gradX, CV_32F, 1, 0, ksize);
		cv::convertScaleAbs(gradX, dst);
	}
	else
	{
		cv::Mat gradY;
		cv::Sobel(gray, gradY, CV_32F, 0, 1, ksize);
		cv::convertScaleAbs(gradY, dst);
	}

	setOutput("output_image", NodeData::createImage(dst));
}
