// nodes/chart/RadarChart.cpp
#include "RadarChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include <QPainter>
#include <QImage>
#include <cmath>

void RadarChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("dimension_columns", "维度列(逗号分隔)", ParamType::String, QString());
	addParam("fill_opacity", "填充透明度", ParamType::DoubleSlider, 0.2,
			 { {"min", 0.0}, {"max", 1.0} });
	addParam("show_legend", "显示图例", ParamType::Bool, true);
}

void RadarChart::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull())
	{
		reportError("输入数据为空"); return;
	}
	auto table = inputData->toDataTable();
	if (!table)
	{
		reportError("无法获取DataTable"); return;
	}

	QStringList dimCols = param("dimension_columns").toString().split(",", Qt::SkipEmptyParts);
	for (auto& s : dimCols) s = s.trimmed();

	if (dimCols.size() < 3)
	{
		reportError("雷达图至少需要3个维度列"); return;
	}

	for (const auto& c : dimCols)
	{
		if (!table->hasColumn(c))
		{
			reportError(QString("列[%1]不存在").arg(c)); return;
		}
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	double fillOpacity = param("fill_opacity").toDouble();

	// 自行用QPainter绘制雷达图
	QImage qimg(w, h, QImage::Format_RGB888);
	qimg.fill(bgColor);
	QPainter painter(&qimg);
	painter.setRenderHint(QPainter::Antialiasing);

	// 标题
	QString title = param("title").toString();
	int topMargin = 30;
	if (!title.isEmpty())
	{
		painter.setFont(QFont(QString(), 14, QFont::Bold));
		painter.setPen(Qt::black);
		painter.drawText(QRect(0, 5, w, 30), Qt::AlignCenter, title);
		topMargin = 40;
	}

	int cx = w / 2;
	int cy = (h + topMargin) / 2;
	int radius = std::min(w, h - topMargin) / 2 - 50;
	int numDim = dimCols.size();
	double angleStep = 2.0 * M_PI / numDim;

	// 找全局最大值做归一化
	double globalMax = 0;
	for (const auto& c : dimCols)
	{
		int idx = table->columnIndex(c);
		for (int r = 0; r < table->rowCount(); ++r)
		{
			bool ok;
			double v = table->value(r, idx).toDouble(&ok);
			if (ok && v > globalMax) globalMax = v;
		}
	}
	if (globalMax < 1e-12) globalMax = 1.0;

	// 绘制网格
	painter.setPen(QPen(QColor(200, 200, 200), 1));
	for (int ring = 1; ring <= 5; ++ring)
	{
		double r = radius * ring / 5.0;
		QPolygonF poly;
		for (int d = 0; d < numDim; ++d)
		{
			double angle = -M_PI / 2.0 + d * angleStep;
			poly << QPointF(cx + r * std::cos(angle), cy + r * std::sin(angle));
		}
		poly << poly.first();
		painter.drawPolyline(poly);
	}

	// 绘制轴线和标签
	painter.setPen(QPen(QColor(150, 150, 150), 1));
	painter.setFont(QFont(QString(), 10));
	for (int d = 0; d < numDim; ++d)
	{
		double angle = -M_PI / 2.0 + d * angleStep;
		double ex = cx + radius * std::cos(angle);
		double ey = cy + radius * std::sin(angle);
		painter.drawLine(QPointF(cx, cy), QPointF(ex, ey));

		// 标签
		double lx = cx + (radius + 20) * std::cos(angle);
		double ly = cy + (radius + 20) * std::sin(angle);
		QRectF labelRect(lx - 50, ly - 10, 100, 20);
		painter.setPen(Qt::black);
		painter.drawText(labelRect, Qt::AlignCenter, dimCols[d]);
		painter.setPen(QPen(QColor(150, 150, 150), 1));
	}

	// 绘制每行数据为一条雷达线
	auto palette = ChartHelper::defaultPalette();

	for (int r = 0; r < table->rowCount(); ++r)
	{
		QPolygonF poly;
		QColor lineColor = palette[r % palette.size()];

		for (int d = 0; d < numDim; ++d)
		{
			int idx = table->columnIndex(dimCols[d]);
			bool ok;
			double v = table->value(r, idx).toDouble(&ok);
			if (!ok) v = 0;
			double normalized = v / globalMax;
			double angle = -M_PI / 2.0 + d * angleStep;
			double px = cx + radius * normalized * std::cos(angle);
			double py = cy + radius * normalized * std::sin(angle);
			poly << QPointF(px, py);
		}
		poly << poly.first();

		QColor fillColor = lineColor;
		fillColor.setAlphaF(fillOpacity);
		painter.setBrush(fillColor);
		painter.setPen(QPen(lineColor, 2));
		painter.drawPolygon(poly);
	}

	painter.end();

	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
