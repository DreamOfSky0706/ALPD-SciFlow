// nodes/drawing/DrawParallelogram.cpp
#include "DrawParallelogram.h"
#include "Utility.h"
#include <QPainter>

void DrawParallelogram::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("x", "左上角X", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("y", "左上角Y", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("width", "宽度", ParamType::Int, 200, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 80, { {"min", 1}, {"max", 20000} });
	addParam("skew", "倾斜量", ParamType::Int, 30, { {"min", 0}, {"max", 200} });
	addParam("fill_color", "填充色", ParamType::Color, QVariantList{ 200, 220, 255, 200 });
	addParam("border_color", "边框色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("border_width", "边框宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 20} });
}

void DrawParallelogram::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	int x = param("x").toInt();
	int y = param("y").toInt();
	int w = param("width").toInt();
	int h = param("height").toInt();
	int sk = param("skew").toInt();

	QPolygonF para;
	para << QPointF(x + sk, y) << QPointF(x + w, y)
		<< QPointF(x + w - sk, y + h) << QPointF(x, y + h);

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(Utility::arrayToColor(param("fill_color").toList()));
	int bw = param("border_width").toInt();
	painter.setPen(bw > 0 ? QPen(Utility::arrayToColor(param("border_color").toList()), bw) : Qt::NoPen);
	painter.drawPolygon(para);
	painter.end();

	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
