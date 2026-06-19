// nodes/geometry/Crop.cpp
#include "Crop.h"

void Crop::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("x", "起点X", ParamType::Int, 0, { {"min", 0}, {"max", 20000} });
	addParam("y", "起点Y", ParamType::Int, 0, { {"min", 0}, {"max", 20000} });
	addParam("crop_width", "裁剪宽度", ParamType::Int, 100, { {"min", 1}, {"max", 20000} });
	addParam("crop_height", "裁剪高度", ParamType::Int, 100, { {"min", 1}, {"max", 20000} });
}

void Crop::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int x = param("x").toInt();
	int y = param("y").toInt();
	int cw = param("crop_width").toInt();
	int ch = param("crop_height").toInt();

	// 与图像边界取交集
	int x1 = std::max(0, x);
	int y1 = std::max(0, y);
	int x2 = std::min(src.cols, x + cw);
	int y2 = std::min(src.rows, y + ch);

	if (x2 <= x1 || y2 <= y1)
	{
		reportError("裁剪区域超出图像范围");
		return;
	}

	if (x1 != x || y1 != y || x2 != x + cw || y2 != y + ch)
	{
		reportWarning("裁剪区域部分越界，已自动裁剪到有效范围");
	}

	cv::Rect roi(x1, y1, x2 - x1, y2 - y1);
	cv::Mat dst = src(roi).clone();
	setOutput("output_image", NodeData::createImage(dst));
}
