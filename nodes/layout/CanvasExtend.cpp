// nodes/layout/CanvasExtend.cpp
#include "CanvasExtend.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void CanvasExtend::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("top", "上方扩展", ParamType::Int, 0, { {"min", 0}, {"max", 2000} });
	addParam("bottom", "下方扩展", ParamType::Int, 0, { {"min", 0}, {"max", 2000} });
	addParam("left", "左侧扩展", ParamType::Int, 0, { {"min", 0}, {"max", 2000} });
	addParam("right", "右侧扩展", ParamType::Int, 0, { {"min", 0}, {"max", 2000} });
	addParam("fill_color", "填充颜色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
}

void CanvasExtend::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	int top = param("top").toInt();
	int bottom = param("bottom").toInt();
	int left = param("left").toInt();
	int right = param("right").toInt();
	QColor fillColor = Utility::arrayToColor(param("fill_color").toList());
	cv::Scalar color = Utility::colorToScalar(fillColor, src.channels() == 4);

	cv::Mat dst;
	cv::copyMakeBorder(src, dst, top, bottom, left, right, cv::BORDER_CONSTANT, color);
	setOutput("output_image", NodeData::createImage(dst));
}
