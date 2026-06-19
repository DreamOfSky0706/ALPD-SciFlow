// nodes/stylize/SketchEffect.cpp
#include "SketchEffect.h"
#include <opencv2/imgproc.hpp>

void SketchEffect::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("line_thickness", "线条粗细", ParamType::IntSlider, 1, { {"min", 1}, {"max", 5} });
	addParam("contrast", "对比度", ParamType::DoubleSlider, 1.5, { {"min", 0.5}, {"max", 3.0} });
}

void SketchEffect::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	cv::Mat gray;
	if (src.channels() >= 3) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else gray = src.clone();

	// 反色
	cv::Mat inverted;
	cv::bitwise_not(gray, inverted);

	// 高斯模糊
	int ksize = param("line_thickness").toInt() * 10 + 1;
	cv::Mat blurred;
	cv::GaussianBlur(inverted, blurred, cv::Size(ksize, ksize), 0);

	// 颜色减淡混合: result = gray / (255 - blurred) * 256
	cv::Mat grayF, blurredF;
	gray.convertTo(grayF, CV_32F);
	blurred.convertTo(blurredF, CV_32F);

	cv::Mat denom = 255.0f - blurredF;
	denom.setTo(1.0, denom < 1.0);

	cv::Mat sketch = grayF / denom * 256.0f;
	sketch.setTo(255.0, sketch > 255.0);

	// 对比度增强
	double contrast = param("contrast").toDouble();
	sketch = sketch * contrast;
	sketch.setTo(255.0, sketch > 255.0);

	cv::Mat dst;
	sketch.convertTo(dst, CV_8U);
	setOutput("output_image", NodeData::createImage(dst));
}
