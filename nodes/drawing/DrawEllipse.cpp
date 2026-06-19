// nodes/drawing/DrawEllipse.cpp
#include "DrawEllipse.h"
#include "Utility.h"
#include <QPainter>

void DrawEllipse::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("center_x", "中心X", ParamType::Int, 200, { {"min", 0}, {"max", 20000} });
	addParam("center_y", "中心Y", ParamType::Int, 150, { {"min", 0}, {"max", 20000} });
	addParam("radius_x", "水平半径", ParamType::Int, 100, { {"min", 1}, {"max", 10000} });
	addParam("radius_y", "垂直半径", ParamType::Int, 60, { {"min", 1}, {"max", 10000} });
	addParam("fill_color", "填充色", ParamType::Color, QVariantList{ 100, 200, 150, 180 });
	addParam("border_color", "边框色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("border_width", "边框宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 20} });
}

void DrawEllipse::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(Utility::arrayToColor(param("fill_color").toList()));
	int bw = param("border_width").toInt();
	painter.setPen(bw > 0 ? QPen(Utility::arrayToColor(param("border_color").toList()), bw) : Qt::NoPen);
	painter.drawEllipse(QPoint(param("center_x").toInt(), param("center_y").toInt()),
						param("radius_x").toInt(), param("radius_y").toInt());
	painter.end();

	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
