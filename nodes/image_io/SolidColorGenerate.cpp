// nodes/image_io/SolidColorGenerate.cpp
#include "SolidColorGenerate.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void SolidColorGenerate::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("size_mode", "尺寸模式", ParamType::Combo, QString("像素"),
			 { {"options", QStringList{"像素", "物理尺寸(mm)+DPI", "物理尺寸(inch)+DPI"}} });
	addParam("width", "宽度", ParamType::Int, 800, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 600, { {"min", 1}, {"max", 20000} });
	addParam("dpi", "DPI", ParamType::Int, 300,
			 { {"min", 72}, {"max", 2400}, {"visible_when", "size_mode!=像素"} });
	addParam("color", "颜色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
}

void SolidColorGenerate::process()
{
	QString sizeMode = param("size_mode").toString();
	int w = param("width").toInt();
	int h = param("height").toInt();
	int dpi = param("dpi").toInt();

	// 换算物理尺寸到像素
	if (sizeMode == "物理尺寸(mm)+DPI")
	{
		w = safeRound((w / 25.4 * dpi));
		h = safeRound((h / 25.4 * dpi));
	}
	else if (sizeMode == "物理尺寸(inch)+DPI")
	{
		w = safeRound((w * dpi));
		h = safeRound((h * dpi));
	}

	if (w < 1 || w > 20000 || h < 1 || h > 20000)
	{
		reportError(QString("图像尺寸超出范围：%1x%2").arg(w).arg(h));
		return;
	}

	QColor color = Utility::arrayToColor(param("color").toList());

	cv::Mat img;
	if (color.alpha() < 255)
	{
		img = cv::Mat(h, w, CV_8UC4, Utility::colorToScalar(color, true));
	}
	else
	{
		img = cv::Mat(h, w, CV_8UC3, Utility::colorToScalar(color, false));
	}

	setOutput("output_image", NodeData::createImage(img));
}
