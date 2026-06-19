// nodes/text/Annotation.cpp
#include "Annotation.h"
#include "Utility.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <cmath>

void Annotation::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	// 标注列表由AnnotationList编辑器控件管理
	// 每项序列化为一个QVariantMap
	addParam("annotations", "标注列表", ParamType::AnnotationList, QVariantList());
}

void Annotation::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QImage qimg = Utility::matToQImage(src);
	if (qimg.format() != QImage::Format_RGB888)
	{
		qimg = qimg.convertToFormat(QImage::Format_RGB888);
	}

	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);

	QVariantList annotations = param("annotations").toList();

	for (const auto& item : annotations)
	{
		QVariantMap ann = item.toMap();
		QString type = ann.value("type", "箭头").toString();
		QColor color = Utility::arrayToColor(ann.value("color", QVariantList{ 255, 0, 0, 255 }).toList());
		int lineWidth = ann.value("line_width", 2).toInt();

		QPen pen(color, lineWidth);
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);

		if (type == "箭头")
		{
			int sx = ann.value("start_x", 0).toInt();
			int sy = ann.value("start_y", 0).toInt();
			int ex = ann.value("end_x", 100).toInt();
			int ey = ann.value("end_y", 100).toInt();

			painter.drawLine(sx, sy, ex, ey);

			// 箭头头部
			double angle = std::atan2(ey - sy, ex - sx);
			int arrowSize = 12;
			QPointF p1(ex - arrowSize * std::cos(angle - 0.4),
					   ey - arrowSize * std::sin(angle - 0.4));
			QPointF p2(ex - arrowSize * std::cos(angle + 0.4),
					   ey - arrowSize * std::sin(angle + 0.4));
			QPolygonF arrowHead;
			arrowHead << QPointF(ex, ey) << p1 << p2;
			painter.setBrush(color);
			painter.drawPolygon(arrowHead);
			painter.setBrush(Qt::NoBrush);
		}
		else if (type == "矩形框")
		{
			int rx = ann.value("rect_x", 0).toInt();
			int ry = ann.value("rect_y", 0).toInt();
			int rw = ann.value("rect_w", 100).toInt();
			int rh = ann.value("rect_h", 100).toInt();
			painter.drawRect(rx, ry, rw, rh);
		}
		else if (type == "圆形框")
		{
			int cx = ann.value("center_x", 50).toInt();
			int cy = ann.value("center_y", 50).toInt();
			int radius = ann.value("radius", 30).toInt();
			painter.drawEllipse(QPoint(cx, cy), radius, radius);
		}
		else if (type == "直线")
		{
			int sx = ann.value("start_x", 0).toInt();
			int sy = ann.value("start_y", 0).toInt();
			int ex = ann.value("end_x", 100).toInt();
			int ey = ann.value("end_y", 100).toInt();
			painter.drawLine(sx, sy, ex, ey);
		}
		else if (type == "文字标签")
		{
			int tx = ann.value("text_x", 0).toInt();
			int ty = ann.value("text_y", 0).toInt();
			QString label = ann.value("text", "").toString();
			int textSize = ann.value("text_size", 14).toInt();
			QFont font(painter.font());
			font.setPointSize(textSize);
			painter.setFont(font);
			painter.drawText(tx, ty, label);
		}
	}

	painter.end();
	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
