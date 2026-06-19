// nodes/chart/ScatterPlot.cpp
#include "ScatterPlot.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"

void ScatterPlot::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("title_font_size", "标题字号", ParamType::IntSlider, 16, { {"min", 8}, {"max", 48} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("x_column", "X轴列", ParamType::String, QString());
	addParam("y_column", "Y轴列", ParamType::String, QString());
	addParam("point_size", "点大小", ParamType::IntSlider, 6, { {"min", 1}, {"max", 30} });
	addParam("default_color", "点颜色", ParamType::Color, QVariantList{ 78, 121, 167, 255 });
	addParam("x_label", "X轴标签", ParamType::String, QString());
	addParam("y_label", "Y轴标签", ParamType::String, QString());
	addParam("show_legend", "显示图例", ParamType::Bool, false);
}

void ScatterPlot::process()
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

	QString xCol = param("x_column").toString();
	QString yCol = param("y_column").toString();
	if (!table->hasColumn(xCol) || !table->hasColumn(yCol))
	{
		reportError("指定的列不存在");
		return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	QColor ptColor = Utility::arrayToColor(param("default_color").toList());
	int ptSize = param("point_size").toInt();

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

	int xIdx = table->columnIndex(xCol);
	int yIdx = table->columnIndex(yCol);

	QCPGraph* graph = plot.addGraph();
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, ptColor, ptColor, ptSize));

	QVector<double> xData, yData;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		bool okx, oky;
		double x = table->value(r, xIdx).toDouble(&okx);
		double y = table->value(r, yIdx).toDouble(&oky);
		if (okx && oky)
		{
			xData.append(x);
			yData.append(y);
		}
	}
	graph->setData(xData, yData);

	plot.xAxis->setLabel(param("x_label").toString());
	plot.yAxis->setLabel(param("y_label").toString());
	if (param("show_legend").toBool()) plot.legend->setVisible(true);
	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
