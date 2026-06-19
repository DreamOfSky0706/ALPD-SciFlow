// nodes/composite/TextWatermark.cpp
#include "TextWatermark.h"
#include "Utility.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QTransform>
#include <opencv2/imgproc.hpp>
#include <cmath>

void TextWatermark::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("text", "水印文字", ParamType::String, QString("WATERMARK"));
	addParam("font_family", "字体", ParamType::Font, QString());
	addParam("font_size", "字号", ParamType::IntSlider, 36, { {"min", 8}, {"max", 200} });
	addParam("color", "颜色", ParamType::Color, QVariantList{ 255, 255, 255, 128 });
	addParam("position", "位置", ParamType::Combo, QString("右下"),
			 { {"options", QStringList{"左上", "上中", "右上", "左中", "正中", "右中",
									  "左下", "下中", "右下"}} });
	addParam("rotation", "旋转角度", ParamType::DoubleSlider, -30.0,
			 { {"min", -90.0}, {"max", 90.0} });
	addParam("tiling", "平铺", ParamType::Bool, false);
}

void TextWatermark::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	if (src.channels() == 1)
	{
		cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);
	}

	QString text = param("text").toString();
	if (text.isEmpty())
	{
		setOutput("output_image", NodeData::createImage(src));
		return;
	}

	QString fontFamily = param("font_family").toString();
	int fontSize = param("font_size").toInt();
	QColor color = Utility::arrayToColor(param("color").toList());
	double rotation = param("rotation").toDouble();
	bool tiling = param("tiling").toBool();

	QFont font(fontFamily, fontSize);
	QFontMetrics fm(font);
	QRect textRect = fm.boundingRect(text);
	int textW = textRect.width() + 20;
	int textH = textRect.height() + 10;

	// 在透明QImage上渲染文字
	QImage textImg(textW, textH, QImage::Format_RGBA8888);
	textImg.fill(Qt::transparent);
	{
		QPainter painter(&textImg);
		painter.setFont(font);
		painter.setPen(color);
		painter.drawText(10, fm.ascent() + 5, text);
	}

	// 旋转文字图像
	if (std::abs(rotation) > 0.1)
	{
		QTransform transform;
		transform.rotate(rotation);
		textImg = textImg.transformed(transform, Qt::SmoothTransformation);
	}

	cv::Mat wmMat = Utility::qImageToMat(textImg);

	// 将水印叠加到主图
	QImage mainImg = Utility::matToQImage(src);
	QImage result = mainImg.convertToFormat(QImage::Format_RGB888);

	QPainter painter(&result);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	QImage wmQImg = Utility::matToQImage(wmMat);

	if (tiling)
	{
		int stepX = wmQImg.width() + 30;
		int stepY = wmQImg.height() + 30;
		for (int ty = -wmQImg.height(); ty < result.height() + wmQImg.height(); ty += stepY)
		{
			for (int tx = -wmQImg.width(); tx < result.width() + wmQImg.width(); tx += stepX)
			{
				painter.drawImage(tx, ty, wmQImg);
			}
		}
	}
	else
	{
		QString pos = param("position").toString();
		int margin = 10;
		int ox = 0;
		int oy = 0;

		if (pos.contains("左")) ox = margin;
		else if (pos.contains("右")) ox = result.width() - wmQImg.width() - margin;
		else ox = (result.width() - wmQImg.width()) / 2;

		if (pos.contains("上")) oy = margin;
		else if (pos.contains("下")) oy = result.height() - wmQImg.height() - margin;
		else oy = (result.height() - wmQImg.height()) / 2;

		painter.drawImage(ox, oy, wmQImg);
	}

	painter.end();

	cv::Mat dstMat = Utility::qImageToMat(result);
	setOutput("output_image", NodeData::createImage(dstMat));
}
