// nodes/edge/AdaptiveThreshold.cpp
#include "AdaptiveThreshold.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void AdaptiveThreshold::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("max_value", "最大值", ParamType::IntSlider, 255, { {"min", 0}, {"max", 255} });
	addParam("method", "自适应方法", ParamType::Combo, QString("高斯(GAUSSIAN_C)"),
			 { {"options", QStringList{"均值(MEAN_C)", "高斯(GAUSSIAN_C)"}} });
	addParam("type", "阈值类型", ParamType::Combo, QString("二值化"),
			 { {"options", QStringList{"二值化", "反二值化"}} });
	addParam("block_size", "块大小", ParamType::IntSlider, 11, { {"min", 3}, {"max", 99} });
	addParam("constant_c", "常数C", ParamType::DoubleSlider, 2.0,
			 { {"min", -50.0}, {"max", 50.0} });
}

void AdaptiveThreshold::process()
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

	int maxVal = param("max_value").toInt();
	int blockSize = Utility::ensureOdd(param("block_size").toInt());
	if (blockSize < 3) blockSize = 3;
	double C = param("constant_c").toDouble();

	QString methodName = param("method").toString();
	int method = cv::ADAPTIVE_THRESH_GAUSSIAN_C;
	if (methodName.startsWith("均值")) method = cv::ADAPTIVE_THRESH_MEAN_C;

	QString typeName = param("type").toString();
	int threshType = cv::THRESH_BINARY;
	if (typeName == "反二值化") threshType = cv::THRESH_BINARY_INV;

	cv::Mat dst;
	cv::adaptiveThreshold(gray, dst, maxVal, method, threshType, blockSize, C);
	setOutput("output_image", NodeData::createImage(dst));
}
