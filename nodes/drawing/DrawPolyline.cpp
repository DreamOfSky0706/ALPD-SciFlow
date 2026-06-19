// nodes/drawing/DrawPolyline.cpp
#include "DrawPolyline.h"
#include "Utility.h"
#include <QPainter>
#include <cmath>

void DrawPolyline::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("points", "坐标点列表", ParamType::PointList, QVariantList());
	addParam("line_color", "线条颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("line_width", "线宽", ParamType::IntSlider, 2, { {"min", 1}, {"max", 10} });
	addParam("arrow_end", "终点箭头", ParamType::Bool, true);
	addParam("arrow_size", "箭头大小", ParamType::IntSlider, 12, { {"min", 5}, {"max", 30} });
	addParam("line_style", "线型", ParamType::Combo, QString("实线"),
			 { {"options", QStringList{"实线", "虚线", "点线"}} });
}

void DrawPolyline::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	QVariantList pts = param("points").toList();
	if (pts.size() < 2)
	{
		reportError("至少需要两个坐标点"); return;
	}

	QColor color = Utility::arrayToColor(param("line_color").toList());
	int lw = param("line_width").toInt();
	QString styleStr = param("line_style").toString();

	Qt::PenStyle penStyle = Qt::SolidLine;
	if (styleStr == "虚线") penStyle = Qt::DashLine;
	else if (styleStr == "点线") penStyle = Qt::DotLine;

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(color, lw, penStyle));

	QVector<QPointF> points;
	for (const auto& p : pts)
	{
		QVariantList xy = p.toList();
		if (xy.size() >= 2) points.append(QPointF(xy[0].toDouble(), xy[1].toDouble()));
	}

	for (int i = 0; i < points.size() - 1; ++i)
	{
		painter.drawLine(points[i], points[i + 1]);
	}

	if (param("arrow_end").toBool() && points.size() >= 2)
	{
		int arrowSize = param("arrow_size").toInt();
		QPointF last = points.last();
		QPointF prev = points[points.size() - 2];
		double angle = std::atan2(last.y() - prev.y(), last.x() - prev.x());
		QPointF p1(last.x() - arrowSize * std::cos(angle - 0.4),
				   last.y() - arrowSize * std::sin(angle - 0.4));
		QPointF p2(last.x() - arrowSize * std::cos(angle + 0.4),
				   last.y() - arrowSize * std::sin(angle + 0.4));
		painter.setBrush(color);
		painter.setPen(Qt::NoPen);
		painter.drawPolygon(QPolygonF() << last << p1 << p2);
	}

	painter.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
