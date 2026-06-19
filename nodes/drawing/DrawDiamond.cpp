// nodes/drawing/DrawDiamond.cpp
#include "DrawDiamond.h"
#include "Utility.h"
#include <QPainter>
#include <QPainterPath>

void DrawDiamond::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("center_x", "中心X", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("center_y", "中心Y", ParamType::Int, 150, { {"min", 0}, {"max", 20000} });
	addParam("half_width", "半宽", ParamType::Int, 80, { {"min", 1}, {"max", 10000} });
	addParam("half_height", "半高", ParamType::Int, 50, { {"min", 1}, {"max", 10000} });
	addParam("fill_color", "填充色", ParamType::Color, QVariantList{ 255, 230, 150, 200 });
	addParam("border_color", "边框色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("border_width", "边框宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 20} });
}

void DrawDiamond::process()
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
	int hw = param("half_width").toInt();
	int hh = param("half_height").toInt();

	QPolygonF diamond;
	diamond << QPointF(cx, cy - hh) << QPointF(cx + hw, cy)
		<< QPointF(cx, cy + hh) << QPointF(cx - hw, cy);

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(Utility::arrayToColor(param("fill_color").toList()));
	int bw = param("border_width").toInt();
	painter.setPen(bw > 0 ? QPen(Utility::arrayToColor(param("border_color").toList()), bw) : Qt::NoPen);
	painter.drawPolygon(diamond);
	painter.end();

	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
