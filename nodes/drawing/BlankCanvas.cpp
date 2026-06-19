// nodes/drawing/BlankCanvas.cpp
#include "BlankCanvas.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void BlankCanvas::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("size_mode", "尺寸模式", ParamType::Combo, QString("像素"),
			 { {"options", QStringList{"像素", "物理尺寸(mm)+DPI", "物理尺寸(inch)+DPI"}} });
	addParam("width", "宽度", ParamType::Int, 800, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 600, { {"min", 1}, {"max", 20000} });
	addParam("dpi", "DPI", ParamType::Int, 300,
			 { {"min", 72}, {"max", 2400}, {"visible_when", "size_mode!=像素"} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("transparent", "透明背景", ParamType::Bool, false);
}

void BlankCanvas::process()
{
	QString sizeMode = param("size_mode").toString();
	int w = param("width").toInt();
	int h = param("height").toInt();
	int dpi = param("dpi").toInt();

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
		reportError(QString("尺寸超出范围：%1x%2").arg(w).arg(h));
		return;
	}

	cv::Mat img;
	if (param("transparent").toBool())
	{
		img = cv::Mat::zeros(h, w, CV_8UC4);
	}
	else
	{
		QColor bg = Utility::arrayToColor(param("background_color").toList());
		if (bg.alpha() < 255)
		{
			img = cv::Mat(h, w, CV_8UC4, Utility::colorToScalar(bg, true));
		}
		else
		{
			img = cv::Mat(h, w, CV_8UC3, Utility::colorToScalar(bg, false));
		}
	}

	setOutput("output_image", NodeData::createImage(img));
}
