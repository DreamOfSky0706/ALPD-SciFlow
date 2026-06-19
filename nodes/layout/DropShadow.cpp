// nodes/layout/DropShadow.cpp
#include "DropShadow.h"
#include "Utility.h"
#include <QPainter>
#include <opencv2/imgproc.hpp>

void DropShadow::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("shadow_color", "阴影颜色", ParamType::Color, QVariantList{ 0, 0, 0, 128 });
	addParam("offset_x", "偏移X", ParamType::IntSlider, 5, { {"min", -50}, {"max", 50} });
	addParam("offset_y", "偏移Y", ParamType::IntSlider, 5, { {"min", -50}, {"max", 50} });
	addParam("blur_radius", "模糊半径", ParamType::IntSlider, 10, { {"min", 0}, {"max", 50} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
}

void DropShadow::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	int ox = param("offset_x").toInt();
	int oy = param("offset_y").toInt();
	int blurR = param("blur_radius").toInt();
	QColor shadowColor = Utility::arrayToColor(param("shadow_color").toList());
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());

	int pad = blurR * 2 + std::max(std::abs(ox), std::abs(oy)) + 10;
	int cw = src.cols + pad * 2;
	int ch = src.rows + pad * 2;

	// 创建阴影层
	cv::Mat shadow = cv::Mat::zeros(ch, cw, CV_8UC4);
	// 在偏移位置填充阴影色块
	int sx = pad + ox;
	int sy = pad + oy;
	cv::Scalar sc(shadowColor.blue(), shadowColor.green(), shadowColor.red(), shadowColor.alpha());
	cv::rectangle(shadow, cv::Rect(sx, sy, src.cols, src.rows), sc, cv::FILLED);

	// 模糊阴影
	if (blurR > 0)
	{
		int ksize = blurR * 2 + 1;
		cv::GaussianBlur(shadow, shadow, cv::Size(ksize, ksize), 0);
	}

	// 合成到背景
	cv::Scalar bg(bgColor.blue(), bgColor.green(), bgColor.red());
	cv::Mat canvas(ch, cw, CV_8UC3, bg);

	// 叠加阴影
	for (int y = 0; y < ch; ++y)
	{
		const uchar* sRow = shadow.ptr<uchar>(y);
		uchar* cRow = canvas.ptr<uchar>(y);
		for (int x = 0; x < cw; ++x)
		{
			double a = sRow[x * 4 + 3] / 255.0;
			for (int c = 0; c < 3; ++c)
			{
				cRow[x * 3 + c] = static_cast<uchar>(
					cRow[x * 3 + c] * (1.0 - a) + sRow[x * 4 + c] * a);
			}
		}
	}

	// 绘制原图
	cv::Mat srcBGR;
	if (src.channels() == 4) cv::cvtColor(src, srcBGR, cv::COLOR_BGRA2BGR);
	else if (src.channels() == 1) cv::cvtColor(src, srcBGR, cv::COLOR_GRAY2BGR);
	else srcBGR = src;

	srcBGR.copyTo(canvas(cv::Rect(pad, pad, src.cols, src.rows)));

	setOutput("output_image", NodeData::createImage(canvas));
}
