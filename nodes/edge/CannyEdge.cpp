// nodes/edge/CannyEdge.cpp
#include "CannyEdge.h"
#include <opencv2/imgproc.hpp>

void CannyEdge::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("threshold_low", "低阈值", ParamType::DoubleSlider, 50.0,
			 { {"min", 0.0}, {"max", 500.0} });
	addParam("threshold_high", "高阈值", ParamType::DoubleSlider, 150.0,
			 { {"min", 0.0}, {"max", 500.0} });
	addParam("aperture_size", "孔径大小", ParamType::Combo, QString("3"),
			 { {"options", QStringList{"3", "5", "7"}} });
}

void CannyEdge::process()
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

	double low = param("threshold_low").toDouble();
	double high = param("threshold_high").toDouble();
	int aperture = param("aperture_size").toString().toInt();

	if (high <= low)
	{
		reportWarning("高阈值应大于低阈值，已自动调整");
		high = low + 1.0;
	}

	cv::Mat edges;
	cv::Canny(gray, edges, low, high, aperture);
	setOutput("output_image", NodeData::createImage(edges));
}
