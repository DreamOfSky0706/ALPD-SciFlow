// nodes/pattern/ChemistryPattern.cpp
#include "ChemistryPattern.h"
#include "Utility.h"
#include <QPainter>
#include <random>
#include <cmath>

void ChemistryPattern::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("pattern_type", "图案类型", ParamType::Combo, QString("分子骨架线框"),
			 { {"options", QStringList{"分子骨架线框", "苯环组合", "元素周期表片段", "化学键图案", "晶体结构"}} });
	addCommonParams();
}

void ChemistryPattern::process()
{
	int w, h, seed; double density; QColor primary, secondary, bg;
	getCommonValues(w, h, primary, secondary, bg, density, seed);
	if (seed == 0) seed = 101;
	std::mt19937 rng(seed);
	QString type = param("pattern_type").toString();

	QImage qimg(w, h, QImage::Format_RGBA8888);
	qimg.fill(bg);
	QPainter p(&qimg);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QPen(primary, param("line_width").toInt()));

	if (type == "苯环组合")
	{
		double r = 25.0 / density;
		double dx = r * 3;
		double dy = r * std::sqrt(3.0) * 1.5;
		for (double y = 0; y < h + dy; y += dy)
			for (double x = 0; x < w + dx; x += dx)
			{
				double ox = x + (static_cast<int>(y / dy) % 2) * r * 1.5;
				QPolygonF hex;
				for (int i = 0; i < 6; ++i)
				{
					double angle = M_PI / 3.0 * i;
					hex << QPointF(ox + r * std::cos(angle), y + r * std::sin(angle));
				}
				p.drawPolygon(hex);
				// 内圆表示共轭
				p.drawEllipse(QPointF(ox, y), r * 0.5, r * 0.5);
			}
	}
	else if (type == "分子骨架线框")
	{
		int count = static_cast<int>(20 * density);
		for (int i = 0; i < count; ++i)
		{
			double cx = rng() % w, cy = rng() % h;
			int bonds = 3 + rng() % 5;
			double angle = (rng() % 360) * M_PI / 180.0;
			double bondLen = 20 + rng() % 20;
			for (int b = 0; b < bonds; ++b)
			{
				double nx = cx + bondLen * std::cos(angle);
				double ny = cy + bondLen * std::sin(angle);
				p.drawLine(QPointF(cx, cy), QPointF(nx, ny));
				p.setBrush(primary); p.drawEllipse(QPointF(cx, cy), 3, 3); p.setBrush(Qt::NoBrush);
				cx = nx; cy = ny;
				angle += (0.8 + (rng() % 100) / 200.0);
			}
		}
	}
	else
	{
		// 简化实现其余类型为网格+文字装饰
		double step = 40.0 / density;
		p.setFont(QFont("Courier", 8));
		QStringList elements = { "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg" };
		int idx = 0;
		for (double y = 10; y < h; y += step)
			for (double x = 10; x < w; x += step)
			{
				p.drawRect(QRectF(x, y, step - 4, step - 4));
				p.drawText(QRectF(x, y, step - 4, step - 4), Qt::AlignCenter, elements[idx % elements.size()]);
				idx++;
			}
	}

	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
