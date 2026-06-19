// nodes/color/BrightnessContrast.cpp
#include "BrightnessContrast.h"

void BrightnessContrast::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("brightness", "亮度", ParamType::IntSlider, 0, { {"min", -100}, {"max", 100} });
	addParam("contrast", "对比度", ParamType::DoubleSlider, 1.0, { {"min", 0.0}, {"max", 3.0} });
}

void BrightnessContrast::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int brightness = param("brightness").toInt();
	double contrast = param("contrast").toDouble();

	cv::Mat dst;
	src.convertTo(dst, -1, contrast, brightness);
	setOutput("output_image", NodeData::createImage(dst));
}
