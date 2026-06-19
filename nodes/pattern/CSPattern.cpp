// nodes/pattern/CSPattern.cpp
#include "CSPattern.h"
#include "Utility.h"
#include <QPainter>
#include <random>

void CSPattern::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("pattern_type", "图案类型", ParamType::Combo, QString("电路板纹路"),
			 { {"options", QStringList{"电路板纹路", "二进制数据流", "网络拓扑", "像素矩阵", "代码块装饰"}} });
	addCommonParams();
}

void CSPattern::process()
{
	int w, h, seed; double density; QColor primary, secondary, bg;
	getCommonValues(w, h, primary, secondary, bg, density, seed);
	if (seed == 0) seed = 42;
	std::mt19937 rng(seed);
	QString type = param("pattern_type").toString();

	QImage qimg(w, h, QImage::Format_RGBA8888);
	qimg.fill(bg);
	QPainter p(&qimg);
	p.setRenderHint(QPainter::Antialiasing);

	if (type == "电路板纹路")
	{
		p.setPen(QPen(primary, 1));
		int step = static_cast<int>(20 / density);
		for (int i = 0; i < static_cast<int>(200 * density); ++i)
		{
			int x = rng() % w; int y = rng() % h;
			int dir = rng() % 2;
			int len = 20 + rng() % 80;
			if (dir == 0) p.drawLine(x, y, x + len, y);
			else p.drawLine(x, y, x, y + len);
			p.setBrush(primary);
			p.drawEllipse(QPoint(x, y), 3, 3);
			p.setBrush(Qt::NoBrush);
		}
	}
	else if (type == "二进制数据流")
	{
		p.setFont(QFont("Courier", static_cast<int>(10 * density)));
		p.setPen(primary);
		int lineH = static_cast<int>(14 * density);
		for (int y = 0; y < h; y += lineH)
		{
			QString line;
			for (int i = 0; i < w / 8; ++i) line += (rng() % 2) ? "1" : "0";
			p.drawText(0, y + lineH, line);
		}
	}
	else if (type == "网络拓扑")
	{
		int nodeCount = static_cast<int>(30 * density);
		QVector<QPointF> nodes;
		for (int i = 0; i < nodeCount; ++i) nodes.append(QPointF(rng() % w, rng() % h));
		p.setPen(QPen(secondary, 1));
		for (int i = 0; i < nodeCount; ++i)
			for (int j = i + 1; j < nodeCount; ++j)
			{
				double dist = QLineF(nodes[i], nodes[j]).length();
				if (dist < 120 / density) p.drawLine(nodes[i], nodes[j]);
			}
		p.setPen(Qt::NoPen); p.setBrush(primary);
		for (const auto& n : nodes) p.drawEllipse(n, 4, 4);
	}
	else if (type == "像素矩阵")
	{
		int sz = static_cast<int>(8 / density);
		if (sz < 2) sz = 2;
		for (int y = 0; y < h; y += sz)
			for (int x = 0; x < w; x += sz)
			{
				int brightness = rng() % 256;
				QColor c(primary.red(), primary.green(), primary.blue(), brightness);
				p.fillRect(x, y, sz - 1, sz - 1, c);
			}
	}
	else
	{
		// 代码块装饰
		int lineH = static_cast<int>(6 * density);
		if (lineH < 3) lineH = 3;
		for (int y = 0; y < h; y += lineH + 2)
		{
			int blockW = 30 + rng() % 150;
			int indent = rng() % 60;
			QColor c = (rng() % 2 == 0) ? primary : secondary;
			c.setAlpha(100 + rng() % 100);
			p.fillRect(indent, y, blockW, lineH, c);
		}
	}

	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
