// nodes/pattern/SocialSciPattern.cpp
#include "SocialSciPattern.h"
#include "Utility.h"
#include <QPainter>
#include <random>
#include <cmath>

void SocialSciPattern::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("pattern_type", "图案类型", ParamType::Combo, QString("社会网络图"),
			 { {"options", QStringList{"社会网络图", "问卷网格", "世界地图轮廓", "统计曲线", "人群图标阵列"}} });
	addCommonParams();
}

void SocialSciPattern::process()
{
	int w, h, seed; double density; QColor primary, secondary, bg;
	getCommonValues(w, h, primary, secondary, bg, density, seed);
	if (seed == 0) seed = 77;
	std::mt19937 rng(seed);
	int lw = param("line_width").toInt();
	QString type = param("pattern_type").toString();

	QImage qimg(w, h, QImage::Format_RGBA8888);
	qimg.fill(bg);
	QPainter p(&qimg);
	p.setRenderHint(QPainter::Antialiasing);

	if (type == "社会网络图")
	{
		int n = static_cast<int>(40 * density);
		QVector<QPointF> nodes;
		for (int i = 0; i < n; ++i) nodes.append(QPointF(rng() % w, rng() % h));
		p.setPen(QPen(secondary, 1));
		for (int i = 0; i < n; ++i)
			for (int j = i + 1; j < n; ++j)
				if (QLineF(nodes[i], nodes[j]).length() < 100 / density)
					p.drawLine(nodes[i], nodes[j]);
		p.setPen(Qt::NoPen);
		for (int i = 0; i < n; ++i)
		{
			int r = 3 + rng() % 6;
			p.setBrush(primary);
			p.drawEllipse(nodes[i], r, r);
		}
	}
	else if (type == "统计曲线")
	{
		p.setPen(QPen(primary, lw + 1));
		// 正态分布曲线
		double mu = w / 2.0, sigma = w / 6.0 / density;
		QPainterPath path;
		double maxY = h * 0.8;
		for (int x = 0; x < w; ++x)
		{
			double z = (x - mu) / sigma;
			double y = maxY * std::exp(-0.5 * z * z);
			if (x == 0) path.moveTo(x, h - y - 20);
			else path.lineTo(x, h - y - 20);
		}
		p.drawPath(path);
		// 基线
		p.setPen(QPen(secondary, 1));
		p.drawLine(0, h - 20, w, h - 20);
	}
	else if (type == "人群图标阵列")
	{
		double step = 30.0 / density;
		p.setPen(QPen(primary, 1));
		p.setBrush(primary);
		for (double y = step; y < h; y += step)
			for (double x = step / 2; x < w; x += step)
			{
				// 简笔画人形
				p.drawEllipse(QPointF(x, y - 8), 4, 4);
				p.drawLine(QPointF(x, y - 4), QPointF(x, y + 6));
				p.drawLine(QPointF(x - 5, y), QPointF(x + 5, y));
				p.drawLine(QPointF(x, y + 6), QPointF(x - 4, y + 12));
				p.drawLine(QPointF(x, y + 6), QPointF(x + 4, y + 12));
			}
	}
	else
	{
		// 问卷网格/其他：简单网格+圆点
		double step = 25.0 / density;
		p.setPen(QPen(secondary, 1));
		for (double y = step; y < h; y += step) p.drawLine(0, y, w, y);
		for (double x = step; x < w; x += step * 4) p.drawLine(x, 0, x, h);
		p.setPen(Qt::NoPen); p.setBrush(primary);
		for (double y = step; y < h; y += step)
			for (double x = step * 2; x < w; x += step * 4)
				if (rng() % 3 == 0) p.drawEllipse(QPointF(x, y), 4, 4);
	}

	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(qimg)));
}
