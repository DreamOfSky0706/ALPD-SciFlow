// nodes/drawing/DrawRectangle.cpp
#include "DrawRectangle.h"
#include "Utility.h"
#include <QPainter>

void DrawRectangle::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("x", "左上角X", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("y", "左上角Y", ParamType::Int, 50, { {"min", 0}, {"max", 20000} });
	addParam("rect_width", "宽度", ParamType::Int, 200, { {"min", 1}, {"max", 20000} });
	addParam("rect_height", "高度", ParamType::Int, 100, { {"min", 1}, {"max", 20000} });
	addParam("fill_color", "填充色", ParamType::Color, QVariantList{ 100, 150, 255, 180 });
	addParam("border_color", "边框色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("border_width", "边框宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 20} });
}

void DrawRectangle::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGBA8888 && qimg.format() != QImage::Format_RGB888)
		qimg = qimg.convertToFormat(QImage::Format_RGBA8888);

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);

	QColor fill = Utility::arrayToColor(param("fill_color").toList());
	QColor border = Utility::arrayToColor(param("border_color").toList());
	int bw = param("border_width").toInt();

	painter.setBrush(fill);
	painter.setPen(bw > 0 ? QPen(border, bw) : Qt::NoPen);
	painter.drawRect(param("x").toInt(), param("y").toInt(),
					 param("rect_width").toInt(), param("rect_height").toInt());
	painter.end();

	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
