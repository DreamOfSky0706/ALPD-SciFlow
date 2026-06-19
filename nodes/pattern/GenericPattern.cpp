// nodes/pattern/GenericPattern.cpp
#include "GenericPattern.h"
#include "Utility.h"
#include <QPainter>
#include <cmath>
#include <random>

void GenericPattern::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("pattern_type", "图案类型", ParamType::Combo, QString("六边形蜂窝网格"),
			 { {"options", QStringList{"六边形蜂窝网格", "同心圆放射线", "网格点阵", "波浪线群组", "三角形铺排", "菱形铺排"}} });
	addCommonParams();
}

void GenericPattern::process()
{
	int w, h, seed; double density; QColor primary, secondary, bg;
	getCommonValues(w, h, primary, secondary, bg, density, seed);
	int lw = param("line_width").toInt();
	QString type = param("pattern_type").toString();

	QImage qimg(w, h, QImage::Format_RGBA8888);
	qimg.fill(bg);
	QPainter p(&qimg);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QPen(primary, lw));

	if (type == "六边形蜂窝网格")
	{
		double r = 30.0 / density;
		double dx = r * 1.5;
		double dy = r * std::sqrt(3.0);
		for (double y = -dy; y < h + dy; y += dy)
			for (double x = -dx; x < w + dx; x += dx * 2)
			{
				double ox = x + (static_cast<int>(y / dy) % 2) * dx;
				QPolygonF hex;
				for (int i = 0; i < 6; ++i)
				{
					double angle = M_PI / 3.0 * i + M_PI / 6.0;
					hex << QPointF(ox + r * std::cos(angle), y + r * std::sin(angle));
				}
				p.drawPolygon(hex);
			}
	}
	else if (type == "同心圆放射线")
	{
		double step = 40.0 / density;
		int cx = w / 2, cy = h / 2;
		double maxR = std::sqrt(w * w + h * h);
		for (double r = step; r < maxR; r += step)
			p.drawEllipse(QPointF(cx, cy), r, r);
		int lines = static_cast<int>(12 * density);
		for (int i = 0; i < lines; ++i)
		{
			double angle = 2.0 * M_PI * i / lines;
			p.drawLine(QPointF(cx, cy), QPointF(cx + maxR * std::cos(angle), cy + maxR * std::sin(angle)));
		}
	}
	else if (type == "网格点阵")
	{
		double step = 20.0 / density;
		double dotR = 2.0;
		p.setBrush(primary);
		p.setPen(Qt::NoPen);
		for (double y = step / 2; y < h; y += step)
			for (double x = step / 2; x < w; x += step)
				p.drawEllipse(QPointF(x, y), dotR, dotR);
	}
	else if (type == "波浪线群组")
	{
		double spacing = 25.0 / density;
		double amp = 10.0;
		for (double y = 0; y < h; y += spacing)
		{
			QPainterPath path;
			path.moveTo(0, y);
			for (double x = 0; x < w; x += 2)
				path.lineTo(x, y + amp * std::sin(x * 0.05 * density));
			p.drawPath(path);
		}
	}
	else
	{
		// 三角形/菱形铺排
		double s = 40.0 / density;
		bool isDiamond = type.contains("菱形");
		for (double y = 0; y < h + s; y += s)
			for (double x = 0; x < w + s; x += s)
			{
				if (isDiamond)
				{
					QPolygonF d;
					d << QPointF(x, y - s / 2) << QPointF(x + s / 2, y) << QPointF(x, y + s / 2) << QPointF(x - s / 2, y);
					p.drawPolygon(d);
				}
				else
				{
					QPolygonF t;
					t << QPointF(x, y) << QPointF(x + s, y) << QPointF(x + s / 2, y - s * 0.866);
					p.drawPolygon(t);
				}
			}
	}

	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
