// nodes/drawing/DrawArrow.cpp
#include "DrawArrow.h"
#include "Utility.h"
#include <QPainter>
#include <cmath>

void DrawArrow::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("start_x", "起点X", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("start_y", "起点Y", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("end_x", "终点X", ParamType::Int, 300, { {"min", 0}, {"max", 20000} });
	addParam("end_y", "终点Y", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("line_color", "线条颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("line_width", "线宽", ParamType::IntSlider, 2, { {"min", 1}, {"max", 10} });
	addParam("arrow_start", "起点箭头", ParamType::Bool, false);
	addParam("arrow_end", "终点箭头", ParamType::Bool, true);
	addParam("arrow_size", "箭头大小", ParamType::IntSlider, 12, { {"min", 5}, {"max", 30} });
}

void DrawArrow::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	int sx = param("start_x").toInt(), sy = param("start_y").toInt();
	int ex = param("end_x").toInt(), ey = param("end_y").toInt();
	QColor color = Utility::arrayToColor(param("line_color").toList());
	int lw = param("line_width").toInt();
	int arrowSize = param("arrow_size").toInt();

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(color, lw));
	painter.drawLine(sx, sy, ex, ey);

	auto drawHead = [&](int tx, int ty, int fx, int fy)
		{
			double angle = std::atan2(ty - fy, tx - fx);
			QPointF p1(tx - arrowSize * std::cos(angle - 0.4),
					   ty - arrowSize * std::sin(angle - 0.4));
			QPointF p2(tx - arrowSize * std::cos(angle + 0.4),
					   ty - arrowSize * std::sin(angle + 0.4));
			QPolygonF head;
			head << QPointF(tx, ty) << p1 << p2;
			painter.setBrush(color);
			painter.setPen(Qt::NoPen);
			painter.drawPolygon(head);
		};

	if (param("arrow_end").toBool()) drawHead(ex, ey, sx, sy);
	if (param("arrow_start").toBool()) drawHead(sx, sy, ex, ey);

	painter.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
