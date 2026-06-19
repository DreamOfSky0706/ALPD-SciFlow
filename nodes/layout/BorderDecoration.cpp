// nodes/layout/BorderDecoration.cpp
#include "BorderDecoration.h"
#include "Utility.h"
#include <QPainter>
#include <opencv2/imgproc.hpp>

void BorderDecoration::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("border_width", "边框宽度", ParamType::IntSlider, 5, { {"min", 0}, {"max", 100} });
	addParam("border_color", "边框颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("corner_radius", "圆角半径", ParamType::IntSlider, 0, { {"min", 0}, {"max", 100} });
	addParam("shadow_enabled", "启用阴影", ParamType::Bool, false);
	addParam("shadow_color", "阴影颜色", ParamType::Color, QVariantList{ 0, 0, 0, 128 });
	addParam("shadow_offset_x", "阴影偏移X", ParamType::IntSlider, 5, { {"min", -30}, {"max", 30} });
	addParam("shadow_offset_y", "阴影偏移Y", ParamType::IntSlider, 5, { {"min", -30}, {"max", 30} });
	addParam("shadow_blur_radius", "阴影模糊", ParamType::IntSlider, 10, { {"min", 0}, {"max", 30} });
}

void BorderDecoration::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	int bw = param("border_width").toInt();
	QColor borderColor = Utility::arrayToColor(param("border_color").toList());
	int cr = param("corner_radius").toInt();
	bool shadowOn = param("shadow_enabled").toBool();

	int shadowExtra = 0;
	if (shadowOn) shadowExtra = param("shadow_blur_radius").toInt() * 2 + 10;

	int totalPad = bw + shadowExtra;
	int canvasW = src.cols + totalPad * 2;
	int canvasH = src.rows + totalPad * 2;

	QImage canvas(canvasW, canvasH, QImage::Format_RGB888);
	canvas.fill(Qt::white);
	QPainter painter(&canvas);
	painter.setRenderHint(QPainter::Antialiasing);

	int imgX = totalPad;
	int imgY = totalPad;

	// 阴影
	if (shadowOn)
	{
		int sox = param("shadow_offset_x").toInt();
		int soy = param("shadow_offset_y").toInt();
		QColor sc = Utility::arrayToColor(param("shadow_color").toList());
		QRect shadowRect(imgX + sox - bw, imgY + soy - bw, src.cols + bw * 2, src.rows + bw * 2);
		painter.setBrush(sc);
		painter.setPen(Qt::NoPen);
		if (cr > 0)
			painter.drawRoundedRect(shadowRect, cr, cr);
		else
			painter.drawRect(shadowRect);

		// 简单模糊：保存到Mat做GaussianBlur再画回来
		painter.end();
		cv::Mat canvasMat = Utility::qImageToMat(canvas);
		int blurR = param("shadow_blur_radius").toInt();
		if (blurR > 0)
		{
			int ksize = blurR * 2 + 1;
			cv::GaussianBlur(canvasMat, canvasMat, cv::Size(ksize, ksize), 0);
		}
		canvas = Utility::matToQImage(canvasMat);
		if (canvas.format() != QImage::Format_RGB888)
			canvas = canvas.convertToFormat(QImage::Format_RGB888);
		painter.begin(&canvas);
		painter.setRenderHint(QPainter::Antialiasing);
	}

	// 边框
	if (bw > 0)
	{
		QRect borderRect(imgX - bw, imgY - bw, src.cols + bw * 2, src.rows + bw * 2);
		painter.setBrush(borderColor);
		painter.setPen(Qt::NoPen);
		if (cr > 0)
			painter.drawRoundedRect(borderRect, cr, cr);
		else
			painter.drawRect(borderRect);
	}

	// 原图
	QImage srcQImg = Utility::matToQImage(src);
	painter.drawImage(imgX, imgY, srcQImg);

	painter.end();
	cv::Mat dst = Utility::qImageToMat(canvas);
	setOutput("output_image", NodeData::createImage(dst));
}
