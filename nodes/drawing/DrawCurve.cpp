// nodes/drawing/DrawCurve.cpp
#include "DrawCurve.h"
#include "Utility.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

void DrawCurve::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("start_x", "起点X", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("start_y", "起点Y", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("end_x", "终点X", ParamType::Int, 350, { {"min", 0}, {"max", 20000} });
	addParam("end_y", "终点Y", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("control1_x", "控制点1 X", ParamType::Int, 100, { {"min", 0}, {"max", 20000} });
	addParam("control1_y", "控制点1 Y", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("control2_x", "控制点2 X", ParamType::Int, 300, { {"min", 0}, {"max", 20000} });
	addParam("control2_y", "控制点2 Y", ParamType::Int, 350, { {"min", 0}, {"max", 20000} });
	addParam("line_color", "线条颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("line_width", "线宽", ParamType::IntSlider, 2, { {"min", 1}, {"max", 10} });
	addParam("arrow_end", "终点箭头", ParamType::Bool, true);
	addParam("arrow_size", "箭头大小", ParamType::IntSlider, 12, { {"min", 5}, {"max", 30} });
}

void DrawCurve::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	QPointF sp(param("start_x").toInt(), param("start_y").toInt());
	QPointF ep(param("end_x").toInt(), param("end_y").toInt());
	QPointF c1(param("control1_x").toInt(), param("control1_y").toInt());
	QPointF c2(param("control2_x").toInt(), param("control2_y").toInt());
	QColor color = Utility::arrayToColor(param("line_color").toList());
	int lw = param("line_width").toInt();

	QPainterPath path;
	path.moveTo(sp);
	path.cubicTo(c1, c2, ep);

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(color, lw));
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(path);

	if (param("arrow_end").toBool())
	{
		int arrowSize = param("arrow_size").toInt();
		// 取曲线末端方向
		double t = 0.99;
		QPointF near = path.pointAtPercent(t);
		double angle = std::atan2(ep.y() - near.y(), ep.x() - near.x());
		QPointF p1(ep.x() - arrowSize * std::cos(angle - 0.4),
				   ep.y() - arrowSize * std::sin(angle - 0.4));
		QPointF p2(ep.x() - arrowSize * std::cos(angle + 0.4),
				   ep.y() - arrowSize * std::sin(angle + 0.4));
		painter.setBrush(color);
		painter.setPen(Qt::NoPen);
		painter.drawPolygon(QPolygonF() << ep << p1 << p2);
	}

	painter.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
