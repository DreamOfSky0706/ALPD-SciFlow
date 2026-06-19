// nodes/chart/PieChart.cpp
#include "PieChart.h"
#include "ChartHelper.h"
#include "DataTable.h"
#include "Utility.h"
#include <QPainter>
#include <QImage>
#include <cmath>
#include <QImage>
#include <cmath>

void PieChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);
	addParam("chart_width", "图表宽度", ParamType::Int, 800, {{"min",200},{"max",4000}});
	addParam("chart_height", "图表高度", ParamType::Int, 600, {{"min",200},{"max",4000}});
	addParam("title", "标题", ParamType::String, QString());
	addParam("title_font_size", "标题字号", ParamType::IntSlider, 16, {{"min",8},{"max",48}});
	addParam("background_color", "背景色", ParamType::Color, QVariantList{255,255,255,255});
	addParam("label_column", "标签列", ParamType::String, QString());
	addParam("value_column", "数值列", ParamType::String, QString());
	addParam("show_percentage", "显示百分比", ParamType::Bool, true);
	addParam("show_legend", "显示图例", ParamType::Bool, true);
	addParam("explode_index", "突出扇区索引(-1无)", ParamType::Int, -1, {{"min",-1},{"max",100}});
}

void PieChart::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull()) { reportError("输入数据为空"); return; }
	auto table = inputData->toDataTable();
	if (!table) { reportError("无法获取DataTable"); return; }

	QString labelCol = param("label_column").toString();
	QString valueCol = param("value_column").toString();
	if (!table->hasColumn(labelCol) || !table->hasColumn(valueCol)) {
		reportError("指定的列不存在"); return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	bool showPct = param("show_percentage").toBool();
	int explodeIdx = param("explode_index").toInt();

	int labelIdx = table->columnIndex(labelCol);
	int valIdx = table->columnIndex(valueCol);
	auto palette = ChartHelper::defaultPalette();

	// 聚合同标签值
	QMap<QString, double> aggVals;
	QStringList labelOrder;
	for (int r = 0; r < table->rowCount(); ++r) {
		QString label = table->value(r, labelIdx).toString();
		bool ok; double val = table->value(r, valIdx).toDouble(&ok);
		if (!ok) val = 0.0;
		if (!aggVals.contains(label)) labelOrder.append(label);
		aggVals[label] += val;
	}

	double total = 0;
	for (auto v : aggVals) total += v;
	if (total <= 0) { reportError("所有数值均为零"); return; }

	QImage img(w, h, QImage::Format_ARGB32);
	img.fill(bgColor);
	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing);

	// 标题
	QString title = param("title").toString();
	int titleFS = param("title_font_size").toInt();
	if (!title.isEmpty()) {
		p.setFont(QFont(QString(), titleFS, QFont::Bold));
		p.setPen(Qt::black);
		p.drawText(QRect(0, 5, w, titleFS + 10), Qt::AlignHCenter | Qt::AlignTop, title);
	}

	// 饼图区域
	int margin = 20;
	int titleH = title.isEmpty() ? 0 : titleFS + 15;
	int chartSize = qMin(w - margin * 2, h - margin * 2 - titleH);
	int cx = w / 2, cy = titleH + (h - titleH) / 2;
	int radius = chartSize / 2 - 20;

	// 图例区域（右侧）
	int legendX = w - 120;
	if (!param("show_legend").toBool()) legendX = w + 10;

	// 绘制饼图扇区
	double startAngle = 90.0 * 16; // QPainter uses 1/16th degree, start from top
	for (int i = 0; i < labelOrder.size(); ++i) {
		const QString& label = labelOrder[i];
		double val = aggVals[label];
		double spanAngle = (val / total) * 360.0 * 16;
		QColor color = palette[i % palette.size()];

		int ex = 0, ey = 0;
		if (i == explodeIdx) {
			double midAngle = (startAngle + spanAngle / 2) / 16.0 * M_PI / 180.0;
			ex = (int)(12 * cos(midAngle));
			ey = -(int)(12 * sin(midAngle));
		}

		p.setBrush(color);
		p.setPen(QPen(color.darker(120), 1));
		p.drawPie(QRect(cx - radius + ex, cy - radius + ey, radius * 2, radius * 2),
				  (int)startAngle, (int)spanAngle);

		// 百分比标签
		if (showPct && spanAngle > 5.0 * 16) {
			double midAngle = (startAngle + spanAngle / 2) / 16.0 * M_PI / 180.0;
			double pct = 100.0 * val / total;
			int lx = cx + ex + (int)((radius * 0.65) * cos(midAngle));
			int ly = cy + ey - (int)((radius * 0.65) * sin(midAngle));
			p.setFont(QFont(QString(), 10));
			p.setPen(Qt::white);
			QString txt = QString("%1%\n%2").arg(pct, 0, 'f', 1).arg(label);
			QRect tr(lx - 40, ly - 18, 80, 36);
			p.drawText(tr, Qt::AlignCenter, txt);
		}

		// 图例
		if (param("show_legend").toBool()) {
			int ly = titleH + 20 + i * 22;
			p.setBrush(color);
			p.setPen(Qt::NoPen);
			p.drawRect(legendX, ly, 14, 14);
			p.setPen(Qt::black);
			p.setFont(QFont(QString(), 10));
			QString leg = QString("%1 (%2%)").arg(label).arg(100.0 * val / total, 0, 'f', 1);
			p.drawText(legendX + 18, ly, 100, 14, Qt::AlignLeft | Qt::AlignVCenter, leg);
		}

		startAngle += spanAngle;
	}

	p.end();
	cv::Mat dst = Utility::qImageToMat(img);
	setOutput("output_image", NodeData::createImage(dst));
}
