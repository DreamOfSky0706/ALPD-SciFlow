// nodes/text/TextOverlay.cpp
#include "TextOverlay.h"
#include "Utility.h"
#include <QPainter>
#include <QFont>
#include <QFontMetricsF>
#include <QPainterPath>
#include <opencv2/imgproc.hpp>

void TextOverlay::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("text", "文本内容", ParamType::MultiLineString, QString());
	addParam("font_family", "字体", ParamType::Font, QString());
	addParam("font_size", "字号", ParamType::IntSlider, 24, { {"min", 8}, {"max", 500} });
	addParam("color", "颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("x", "位置X", ParamType::Int, 10, { {"min", 0}, {"max", 20000} });
	addParam("y", "位置Y", ParamType::Int, 30, { {"min", 0}, {"max", 20000} });
	addParam("alignment", "对齐方式", ParamType::Combo, QString("左对齐"),
			 { {"options", QStringList{"左对齐", "居中", "右对齐"}} });
	addParam("line_spacing", "行间距倍数", ParamType::DoubleSlider, 1.2,
			 { {"min", 0.5}, {"max", 3.0} });
	addParam("stroke_color", "描边颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("stroke_width", "描边宽度", ParamType::IntSlider, 0, { {"min", 0}, {"max", 10} });
}

void TextOverlay::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QString text = param("text").toString();
	if (text.isEmpty())
	{
		setOutput("output_image", NodeData::createImage(src));
		return;
	}

	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGB888 && qimg.format() != QImage::Format_RGBA8888)
	{
		qimg = qimg.convertToFormat(QImage::Format_RGB888);
	}

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	QString fontFamily = param("font_family").toString();
	int fontSize = param("font_size").toInt();
	QFont font(fontFamily, fontSize);
	painter.setFont(font);

	QColor textColor = Utility::arrayToColor(param("color").toList());
	QColor strokeColor = Utility::arrayToColor(param("stroke_color").toList());
	int strokeWidth = param("stroke_width").toInt();
	int posX = param("x").toInt();
	int posY = param("y").toInt();
	double lineSpacing = param("line_spacing").toDouble();
	QString alignment = param("alignment").toString();

	QFontMetricsF fm(font);
	double lineHeight = fm.height() * lineSpacing;

	QStringList lines = text.split("\n");

	Qt::AlignmentFlag hAlign = Qt::AlignLeft;
	if (alignment == "居中") hAlign = Qt::AlignHCenter;
	else if (alignment == "右对齐") hAlign = Qt::AlignRight;

	for (int i = 0; i < lines.size(); ++i)
	{
		double ly = posY + i * lineHeight;

		if (strokeWidth > 0)
		{
			QPainterPath path;
			double lx = posX;
			if (hAlign == Qt::AlignHCenter)
			{
				lx = posX - fm.horizontalAdvance(lines[i]) / 2.0;
			}
			else if (hAlign == Qt::AlignRight)
			{
				lx = posX - fm.horizontalAdvance(lines[i]);
			}
			path.addText(lx, ly + fm.ascent(), font, lines[i]);
			painter.setPen(QPen(strokeColor, strokeWidth * 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
			painter.drawPath(path);
			painter.setPen(Qt::NoPen);
			painter.setBrush(textColor);
			painter.drawPath(path);
		}
		else
		{
			painter.setPen(textColor);
			QRectF rect(posX, ly, qimg.width() - posX, lineHeight);
			painter.drawText(rect, hAlign | Qt::AlignTop, lines[i]);
		}
	}

	painter.end();

	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
