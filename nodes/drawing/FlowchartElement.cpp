// nodes/drawing/FlowchartElement.cpp
#include "FlowchartElement.h"
#include "Utility.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>

void FlowchartElement::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("shape", "形状", ParamType::Combo, QString("圆角矩形"),
			 { {"options", QStringList{"圆角矩形", "矩形", "菱形", "平行四边形",
									  "椭圆", "圆形", "六边形", "梯形"}} });
	addParam("center_x", "中心X", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("center_y", "中心Y", ParamType::Int, 150, { {"min", 0}, {"max", 20000} });
	addParam("elem_width", "元素宽度", ParamType::Int, 120, { {"min", 20}, {"max", 2000} });
	addParam("elem_height", "元素高度", ParamType::Int, 60, { {"min", 20}, {"max", 2000} });
	addParam("text", "文本", ParamType::String, QString("步骤"));
	addParam("text_font_size", "文字字号", ParamType::IntSlider, 14, { {"min", 8}, {"max", 48} });
	addParam("text_color", "文字颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("fill_color", "填充色", ParamType::Color, QVariantList{ 200, 220, 255, 255 });
	addParam("border_color", "边框色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("border_width", "边框宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 10} });
}

void FlowchartElement::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	int cx = param("center_x").toInt();
	int cy = param("center_y").toInt();
	int ew = param("elem_width").toInt();
	int eh = param("elem_height").toInt();
	QString shape = param("shape").toString();
	QColor fill = Utility::arrayToColor(param("fill_color").toList());
	QColor border = Utility::arrayToColor(param("border_color").toList());
	int bw = param("border_width").toInt();

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(fill);
	painter.setPen(bw > 0 ? QPen(border, bw) : Qt::NoPen);

	int x = cx - ew / 2, y = cy - eh / 2;

	if (shape == "圆角矩形")
	{
		painter.drawRoundedRect(x, y, ew, eh, 10, 10);
	}
	else if (shape == "矩形")
	{
		painter.drawRect(x, y, ew, eh);
	}
	else if (shape == "菱形")
	{
		QPolygonF d;
		d << QPointF(cx, y) << QPointF(cx + ew / 2, cy)
			<< QPointF(cx, y + eh) << QPointF(cx - ew / 2, cy);
		painter.drawPolygon(d);
	}
	else if (shape == "平行四边形")
	{
		int sk = ew / 6;
		QPolygonF p;
		p << QPointF(x + sk, y) << QPointF(x + ew, y)
			<< QPointF(x + ew - sk, y + eh) << QPointF(x, y + eh);
		painter.drawPolygon(p);
	}
	else if (shape == "椭圆" || shape == "圆形")
	{
		painter.drawEllipse(QPoint(cx, cy), ew / 2, eh / 2);
	}
	else if (shape == "六边形")
	{
		int s = ew / 4;
		QPolygonF hex;
		hex << QPointF(x + s, y) << QPointF(x + ew - s, y) << QPointF(x + ew, cy)
			<< QPointF(x + ew - s, y + eh) << QPointF(x + s, y + eh) << QPointF(x, cy);
		painter.drawPolygon(hex);
	}
	else if (shape == "梯形")
	{
		int s = ew / 5;
		QPolygonF trap;
		trap << QPointF(x + s, y) << QPointF(x + ew - s, y)
			<< QPointF(x + ew, y + eh) << QPointF(x, y + eh);
		painter.drawPolygon(trap);
	}

	// 文字
	QString text = param("text").toString();
	if (!text.isEmpty())
	{
		QColor tc = Utility::arrayToColor(param("text_color").toList());
		int fs = param("text_font_size").toInt();
		painter.setFont(QFont(QString(), fs));
		painter.setPen(tc);
		painter.drawText(QRect(x + 4, y + 2, ew - 8, eh - 4), Qt::AlignCenter | Qt::TextWordWrap, text);
	}

	painter.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
