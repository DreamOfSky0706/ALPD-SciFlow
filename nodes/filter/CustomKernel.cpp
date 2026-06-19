// nodes/filter/CustomKernel.cpp
#include "CustomKernel.h"
#include <opencv2/imgproc.hpp>

void CustomKernel::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("kernel_size", "核大小", ParamType::Combo, QString("3x3"),
			 { {"options", QStringList{"3x3", "5x5", "7x7"}} });

	// 默认3x3单位核
	QVariantList defaultKernel;
	for (int i = 0; i < 9; ++i)
	{
		defaultKernel << ((i == 4) ? 1.0 : 0.0);
	}
	addParam("kernel_values", "核矩阵", ParamType::KernelMatrix, defaultKernel);
	addParam("normalize", "归一化", ParamType::Bool, true);
}

void CustomKernel::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QString sizeStr = param("kernel_size").toString();

	int ksize = 3;
	if (sizeStr == "5x5") ksize = 5;
	else if (sizeStr == "7x7") ksize = 7;

	QVariantList vals = param("kernel_values").toList();
	int expected = ksize * ksize;

	cv::Mat kernel(ksize, ksize, CV_32F);

	double sum = 0.0;
	for (int i = 0; i < expected; ++i)
	{
		float v = 0.0f;
		if (i < vals.size())
		{
			v = vals[i].toFloat();
		}
		kernel.at<float>(i / ksize, i % ksize) = v;
		sum += v;
	}

	// 归一化
	if (param("normalize").toBool() && std::abs(sum) > 1e-6)
	{
		kernel /= static_cast<float>(sum);
	}

	cv::Mat dst;
	cv::filter2D(src, dst, -1, kernel);
	setOutput("output_image", NodeData::createImage(dst));
}
