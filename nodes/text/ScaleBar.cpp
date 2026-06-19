// nodes/text/ScaleBar.cpp
#include "ScaleBar.h"
#include "Utility.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

void ScaleBar::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("pixel_per_unit", "每像素代表的实际长度", ParamType::Double, 1.0,
			 { {"min", 0.0001}, {"max", 1000000.0} });
	addParam("unit", "单位", ParamType::Combo, QString("μm"),
			 { {"options", QStringList{"nm", "μm", "mm", "cm", "m", "km"}} });
	addParam("bar_length_unit", "比例尺表示的长度", ParamType::Double, 10.0,
			 { {"min", 0.001}, {"max", 1000000.0} });
	addParam("position", "位置", ParamType::Combo, QString("左下"),
			 { {"options", QStringList{"左上", "右上", "左下", "右下"}} });
	addParam("bar_color", "比例尺颜色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("bar_height", "比例尺高度", ParamType::IntSlider, 5, { {"min", 2}, {"max", 20} });
	addParam("font_size", "字号", ParamType::IntSlider, 14, { {"min", 8}, {"max", 50} });
	addParam("background_enabled", "显示背景", ParamType::Bool, true);
}

void ScaleBar::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	double pixPerUnit = param("pixel_per_unit").toDouble();
	double barLengthUnit = param("bar_length_unit").toDouble();
	QString unit = param("unit").toString();
	int barHeight = param("bar_height").toInt();
	int fontSize = param("font_size").toInt();
	QColor barColor = Utility::arrayToColor(param("bar_color").toList());
	bool bgEnabled = param("background_enabled").toBool();
	QString position = param("position").toString();

	// 计算像素长度
	double barPixels = barLengthUnit / pixPerUnit;

	if (barPixels < 10)
	{
		reportWarning("比例尺像素长度过短，请检查参数");
	}
	if (barPixels > src.cols * 0.8)
	{
		reportWarning("比例尺像素长度超过图像宽度的80%，请检查参数");
	}

	int barW = static_cast<int>(std::round(barPixels));

	// 标注文字
	QString label = QString("%1 %2").arg(barLengthUnit).arg(unit);

	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGB888)
	{
		qimg = qimg.convertToFormat(QImage::Format_RGB888);
	}

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);

	QFont font(painter.font());
	font.setPointSize(fontSize);
	painter.setFont(font);
	QFontMetrics fm(font);

	int labelW = fm.horizontalAdvance(label);
	int totalW = std::max(barW, labelW) + 16;
	int totalH = barHeight + fm.height() + 12;

	// 确定位置
	int margin = 15;
	int ox, oy;

	if (position.contains("右"))
	{
		ox = qimg.width() - totalW - margin;
	}
	else
	{
		ox = margin;
	}

	if (position.contains("下"))
	{
		oy = qimg.height() - totalH - margin;
	}
	else
	{
		oy = margin;
	}

	// 背景
	if (bgEnabled)
	{
		painter.fillRect(ox - 4, oy - 4, totalW + 8, totalH + 8,
						 QColor(0, 0, 0, 128));
	}

	// 比例尺矩形条
	int barX = ox + (totalW - barW) / 2;
	int barY = oy + fm.height() + 4;
	painter.fillRect(barX, barY, barW, barHeight, barColor);

	// 两端竖线端点
	painter.setPen(QPen(barColor, 2));
	int tickH = barHeight + 6;
	painter.drawLine(barX, barY - 3, barX, barY + barHeight + 3);          // 左端点
	painter.drawLine(barX + barW, barY - 3, barX + barW, barY + barHeight + 3); // 右端点

	// 标注文字
	painter.setPen(barColor);
	int textX = ox + (totalW - labelW) / 2;
	int textY = oy + fm.ascent();
	painter.drawText(textX, textY, label);

	painter.end();
	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
