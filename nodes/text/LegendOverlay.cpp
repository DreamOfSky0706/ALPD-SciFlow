// nodes/text/LegendOverlay.cpp
#include "LegendOverlay.h"
#include "Utility.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>

void LegendOverlay::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	// 图例项列表，每项为{color:[r,g,b,a], label:"text", style:"方块/圆形/线条"}
	addParam("items", "图例项", ParamType::LegendList, QVariantList());
	addParam("position", "位置", ParamType::Combo, QString("右上"),
			 { {"options", QStringList{"左上", "右上", "左下", "右下"}} });
	addParam("font_size", "字号", ParamType::IntSlider, 12, { {"min", 8}, {"max", 36} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 220 });
	addParam("border_enabled", "显示边框", ParamType::Bool, true);
}

void LegendOverlay::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QVariantList items = param("items").toList();

	if (items.isEmpty())
	{
		setOutput("output_image", NodeData::createImage(src));
		return;
	}

	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGB888)
	{
		qimg = qimg.convertToFormat(QImage::Format_RGB888);
	}

	int fontSize = param("font_size").toInt();
	QFont font(qimg.text("font"), fontSize);
	font.setPointSize(fontSize);
	QFontMetrics fm(font);

	int iconSize = fm.height();
	int itemHeight = iconSize + 4;
	int padding = 8;
	int spacing = 6;

	// 计算图例框尺寸
	int maxLabelW = 0;
	for (const auto& item : items)
	{
		QVariantMap m = item.toMap();
		QString label = m.value("label", "").toString();
		maxLabelW = std::max(maxLabelW, fm.horizontalAdvance(label));
	}
	int boxW = padding * 2 + iconSize + spacing + maxLabelW;
	int boxH = padding * 2 + items.size() * itemHeight - 4;

	QString position = param("position").toString();
	int margin = 10;
	int ox, oy;

	if (position.contains("右")) ox = qimg.width() - boxW - margin;
	else ox = margin;

	if (position.contains("下")) oy = qimg.height() - boxH - margin;
	else oy = margin;

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);

	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	painter.fillRect(ox, oy, boxW, boxH, bgColor);

	if (param("border_enabled").toBool())
	{
		painter.setPen(QPen(Qt::black, 1));
		painter.drawRect(ox, oy, boxW, boxH);
	}

	painter.setFont(font);

	for (int i = 0; i < items.size(); ++i)
	{
		QVariantMap m = items[i].toMap();
		QColor color = Utility::arrayToColor(m.value("color", QVariantList{ 0, 0, 0, 255 }).toList());
		QString label = m.value("label", "").toString();
		QString style = m.value("style", "方块").toString();

		int iy = oy + padding + i * itemHeight;
		int ix = ox + padding;

		if (style == "圆形")
		{
			painter.setBrush(color);
			painter.setPen(Qt::NoPen);
			painter.drawEllipse(ix, iy, iconSize, iconSize);
		}
		else if (style == "线条")
		{
			painter.setPen(QPen(color, 3));
			painter.drawLine(ix, iy + iconSize / 2, ix + iconSize, iy + iconSize / 2);
		}
		else
		{
			painter.fillRect(ix, iy, iconSize, iconSize, color);
		}

		painter.setPen(Qt::black);
		painter.drawText(ix + iconSize + spacing, iy + fm.ascent(), label);
	}

	painter.end();
	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
