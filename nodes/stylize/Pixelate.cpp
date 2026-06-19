// nodes/stylize/Pixelate.cpp
#include "Pixelate.h"
#include <opencv2/imgproc.hpp>

void Pixelate::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("block_size", "方块大小", ParamType::IntSlider, 10, { {"min", 2}, {"max", 100} });
}

void Pixelate::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	int blockSize = param("block_size").toInt();

	int smallW = std::max(1, src.cols / blockSize);
	int smallH = std::max(1, src.rows / blockSize);

	cv::Mat small;
	cv::resize(src, small, cv::Size(smallW, smallH), 0, 0, cv::INTER_LINEAR);

	cv::Mat dst;
	cv::resize(small, dst, src.size(), 0, 0, cv::INTER_NEAREST);
	setOutput("output_image", NodeData::createImage(dst));
}
