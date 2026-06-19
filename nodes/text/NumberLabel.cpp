// nodes/text/NumberLabel.cpp
#include "NumberLabel.h"
#include "Utility.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>

void NumberLabel::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("label_text", "标签内容", ParamType::String, QString("(a)"));
	addParam("position", "位置", ParamType::Combo, QString("左上"),
			 { {"options", QStringList{"左上", "右上", "左下", "右下"}} });
	addParam("font_family", "字体", ParamType::Font, QString());
	addParam("font_size", "字号", ParamType::IntSlider, 18, { {"min", 8}, {"max", 72} });
	addParam("text_color", "文字颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("background_color", "背景颜色", ParamType::Color, QVariantList{ 255, 255, 255, 200 });
	addParam("padding", "内边距", ParamType::IntSlider, 4, { {"min", 0}, {"max", 20} });
}

void NumberLabel::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGB888 && qimg.format() != QImage::Format_RGBA8888)
	{
		qimg = qimg.convertToFormat(QImage::Format_RGB888);
	}

	QString labelText = param("label_text").toString();
	QString fontFamily = param("font_family").toString();
	int fontSize = param("font_size").toInt();
	QColor textColor = Utility::arrayToColor(param("text_color").toList());
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	int padding = param("padding").toInt();
	QString position = param("position").toString();

	QFont font(fontFamily, fontSize);
	QFontMetrics fm(font);
	int textW = fm.horizontalAdvance(labelText);
	int textH = fm.height();
	int boxW = textW + padding * 2;
	int boxH = textH + padding * 2;

	int margin = 5;
	int ox, oy;

	if (position.contains("右")) ox = qimg.width() - boxW - margin;
	else ox = margin;

	if (position.contains("下")) oy = qimg.height() - boxH - margin;
	else oy = margin;

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(ox, oy, boxW, boxH, bgColor);
	painter.setFont(font);
	painter.setPen(textColor);
	painter.drawText(ox + padding, oy + padding + fm.ascent(), labelText);
	painter.end();

	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
