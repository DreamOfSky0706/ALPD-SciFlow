// nodes/filter/MotionBlur.cpp
#include "MotionBlur.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void MotionBlur::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("angle", "运动方向(度)", ParamType::DoubleSlider, 0.0,
			 { {"min", 0.0}, {"max", 360.0} });
	addParam("length", "运动长度", ParamType::IntSlider, 15, { {"min", 1}, {"max", 200} });
}

void MotionBlur::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	double angleDeg = param("angle").toDouble();
	int length = param("length").toInt();
	if (length < 1)
	{
		length = 1;
	}

	// 构造运动模糊核
	cv::Mat kernel = cv::Mat::zeros(length, length, CV_32F);
	double angleRad = angleDeg * CV_PI / 180.0;
	double cosA = std::cos(angleRad);
	double sinA = std::sin(angleRad);

	int center = length / 2;
	int pointCount = 0;

	for (int i = 0; i < length; ++i)
	{
		double offset = i - center;
		int kx = center + static_cast<int>(std::round(offset * cosA));
		int ky = center + static_cast<int>(std::round(offset * sinA));

		if (kx >= 0 && kx < length && ky >= 0 && ky < length)
		{
			kernel.at<float>(ky, kx) = 1.0f;
			pointCount++;
		}
	}

	// 归一化
	if (pointCount > 0)
	{
		kernel /= static_cast<float>(pointCount);
	}

	cv::Mat dst;
	cv::filter2D(src, dst, -1, kernel);
	setOutput("output_image", NodeData::createImage(dst));
}
