// nodes/utility/ImageInfo.cpp
#include "ImageInfo.h"

void ImageInfo::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_width", DataType::Numeric);
	addOutputPort("output_height", DataType::Numeric);
	addOutputPort("output_channels", DataType::Numeric);
	addOutputPort("output_summary", DataType::Text);
}

void ImageInfo::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat img = inputData->toImage();
	setOutput("output_width", NodeData::createNumeric(img.cols));
	setOutput("output_height", NodeData::createNumeric(img.rows));
	setOutput("output_channels", NodeData::createNumeric(img.channels()));

	QString depthStr = (img.depth() == CV_8U) ? "8位" : (img.depth() == CV_16U) ? "16位" : "其他";
	QString summary = QString("%1x%2, %3通道, %4").arg(img.cols).arg(img.rows).arg(img.channels()).arg(depthStr);
	setOutput("output_summary", NodeData::createText(summary));
}
