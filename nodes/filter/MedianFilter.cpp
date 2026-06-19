// nodes/filter/MedianFilter.cpp
#include "MedianFilter.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void MedianFilter::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("kernel_size", "核大小", ParamType::IntSlider, 5, { {"min", 3}, {"max", 99} });
}

void MedianFilter::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int ksize = Utility::ensureOdd(param("kernel_size").toInt());
	if (ksize < 3)
	{
		ksize = 3;
	}

	cv::Mat dst;
	cv::medianBlur(src, dst, ksize);
	setOutput("output_image", NodeData::createImage(dst));
}
