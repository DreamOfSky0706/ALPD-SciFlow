// nodes/chart/BoxPlot.cpp
#include "BoxPlot.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"
#include <algorithm>

void BoxPlot::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("value_column", "数值列", ParamType::String, QString());
	addParam("category_column", "分组列(可选)", ParamType::String, QString());
	addParam("box_color", "箱体颜色", ParamType::Color, QVariantList{ 78, 121, 167, 255 });
}

void BoxPlot::process()
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

	QString valCol = param("value_column").toString();
	if (!table->hasColumn(valCol))
	{
		reportError(QString("列[%1]不存在").arg(valCol)); return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	QColor boxColor = Utility::arrayToColor(param("box_color").toList());

	int valIdx = table->columnIndex(valCol);
	QString catCol = param("category_column").toString().trimmed();

	// 按组收集数据
	QMap<QString, QVector<double>> groups;
	QStringList groupOrder;

	if (!catCol.isEmpty() && table->hasColumn(catCol))
	{
		int catIdx = table->columnIndex(catCol);
		for (int r = 0; r < table->rowCount(); ++r)
		{
			QString key = table->value(r, catIdx).toString();
			if (!groups.contains(key)) groupOrder.append(key);
			bool ok;
			double v = table->value(r, valIdx).toDouble(&ok);
			if (ok) groups[key].append(v);
		}
	}
	else
	{
		groupOrder.append(valCol);
		for (int r = 0; r < table->rowCount(); ++r)
		{
			bool ok;
			double v = table->value(r, valIdx).toDouble(&ok);
			if (ok) groups[valCol].append(v);
		}
	}

	QCustomPlot plot;
	plot.setBackground(QBrush(bgColor));
	plot.setFixedSize(w, h);

	QString title = param("title").toString();
	if (!title.isEmpty())
	{
		plot.plotLayout()->insertRow(0);
		auto* te = new QCPTextElement(&plot, title);
		te->setFont(QFont(QString(), 14, QFont::Bold));
		plot.plotLayout()->addElement(0, 0, te);
	}

	QCPStatisticalBox* statBox = new QCPStatisticalBox(plot.xAxis, plot.yAxis);
	statBox->setBrush(boxColor);
	statBox->setPen(QPen(boxColor.darker(150)));
	statBox->setMedianPen(QPen(Qt::black, 2));

	QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);

	for (int g = 0; g < groupOrder.size(); ++g)
	{
		QVector<double> sorted = groups[groupOrder[g]];
		if (sorted.isEmpty()) continue;
		std::sort(sorted.begin(), sorted.end());

		int n = sorted.size();
		double minV = sorted.first();
		double maxV = sorted.last();
		double q1 = sorted[n / 4];
		double median = sorted[n / 2];
		double q3 = sorted[3 * n / 4];

		// IQR whiskers
		double iqr = q3 - q1;
		double wLo = std::max(minV, q1 - 1.5 * iqr);
		double wHi = std::min(maxV, q3 + 1.5 * iqr);

		statBox->addData(g + 1, wLo, q1, median, q3, wHi);
		ticker->addTick(g + 1, groupOrder[g]);
	}

	plot.xAxis->setTicker(ticker);
	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
