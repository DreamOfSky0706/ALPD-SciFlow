// nodes/pattern/BioPattern.cpp
#include "BioPattern.h"
#include "Utility.h"
#include <QPainter>
#include <cmath>

void BioPattern::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("pattern_type", "图案类型", ParamType::Combo, QString("DNA双螺旋"),
			 { {"options", QStringList{"DNA双螺旋", "磷脂双分子层", "蛋白质二级结构", "神经元连接", "细胞分裂"}} });
	addCommonParams();
}

void BioPattern::process()
{
	int w, h, seed; double density; QColor primary, secondary, bg;
	getCommonValues(w, h, primary, secondary, bg, density, seed);
	int lw = param("line_width").toInt();
	QString type = param("pattern_type").toString();

	QImage qimg(w, h, QImage::Format_RGBA8888);
	qimg.fill(bg);
	QPainter p(&qimg);
	p.setRenderHint(QPainter::Antialiasing);

	if (type == "DNA双螺旋")
	{
		double amp = 40.0 * density;
		double freq = 0.02 * density;
		double cx = w / 2.0;
		p.setPen(QPen(primary, lw + 1));
		QPainterPath path1, path2;
		for (int y = 0; y < h; ++y)
		{
			double x1 = cx + amp * std::sin(freq * y);
			double x2 = cx + amp * std::sin(freq * y + M_PI);
			if (y == 0)
			{
				path1.moveTo(x1, y); path2.moveTo(x2, y);
			}
			else
			{
				path1.lineTo(x1, y); path2.lineTo(x2, y);
			}
			// 碱基连线
			if (y % static_cast<int>(15 / density) == 0)
			{
				p.setPen(QPen(secondary, 1));
				p.drawLine(QPointF(x1, y), QPointF(x2, y));
				p.setPen(QPen(primary, lw + 1));
			}
		}
		p.drawPath(path1);
		p.setPen(QPen(secondary, lw + 1));
		p.drawPath(path2);
	}
	else
	{
		// 简化实现：画一些生物形态的装饰圆和线
		p.setPen(QPen(primary, lw));
		double step = 60.0 / density;
		for (double y = step; y < h; y += step)
			for (double x = step; x < w; x += step)
			{
				p.drawEllipse(QPointF(x, y), step * 0.3, step * 0.3);
				if (x + step < w) p.drawLine(QPointF(x + step * 0.3, y), QPointF(x + step - step * 0.3, y));
			}
	}

	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
