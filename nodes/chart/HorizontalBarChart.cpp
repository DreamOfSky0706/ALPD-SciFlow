// nodes/chart/HorizontalBarChart.cpp
#include "HorizontalBarChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"

void HorizontalBarChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("title_font_size", "标题字号", ParamType::IntSlider, 16, { {"min", 8}, {"max", 48} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("category_column", "分类列", ParamType::String, QString());
	addParam("value_column", "数值列", ParamType::String, QString());
	addParam("x_label", "X轴标签", ParamType::String, QString());
	addParam("y_label", "Y轴标签", ParamType::String, QString());
	addParam("show_legend", "显示图例", ParamType::Bool, false);
}

void HorizontalBarChart::process()
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

	QString catCol = param("category_column").toString();
	QString valCol = param("value_column").toString();
	if (!table->hasColumn(catCol) || !table->hasColumn(valCol))
	{
		reportError("指定的列不存在");
		return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());

	QCustomPlot plot;
	plot.setBackground(QBrush(bgColor));
	plot.setFixedSize(w, h);

	QString title = param("title").toString();
	if (!title.isEmpty())
	{
		plot.plotLayout()->insertRow(0);
		auto* te = new QCPTextElement(&plot, title);
		te->setFont(QFont(QString(), param("title_font_size").toInt(), QFont::Bold));
		plot.plotLayout()->addElement(0, 0, te);
	}

	// 水平条形图：将key轴设为yAxis，value轴设为xAxis
	QCPBars* bars = new QCPBars(plot.yAxis, plot.xAxis);
	bars->setWidth(0.7);

	int catIdx = table->columnIndex(catCol);
	int valIdx = table->columnIndex(valCol);
	auto palette = ChartHelper::defaultPalette();
	bars->setPen(QPen(palette[0].darker(120)));
	bars->setBrush(palette[0]);

	QVector<double> ticks;
	QVector<double> values;
	QVector<QString> labels;

	for (int r = 0; r < table->rowCount(); ++r)
	{
		ticks.append(r + 1);
		labels.append(table->value(r, catIdx).toString());
		bool ok;
		double v = table->value(r, valIdx).toDouble(&ok);
		values.append(ok ? v : 0.0);
	}

	bars->setData(ticks, values);

	QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
	for (int i = 0; i < labels.size(); ++i)
	{
		ticker->addTick(ticks[i], labels[i]);
	}
	plot.yAxis->setTicker(ticker);
	plot.xAxis->setLabel(param("x_label").toString());
	plot.yAxis->setLabel(param("y_label").toString());

	if (param("show_legend").toBool()) plot.legend->setVisible(true);
	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
