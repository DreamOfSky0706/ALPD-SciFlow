// nodes/chart/AreaChart.cpp
#include "AreaChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"

void AreaChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("title_font_size", "标题字号", ParamType::IntSlider, 16, { {"min", 8}, {"max", 48} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("x_column", "X轴列", ParamType::String, QString());
	addParam("y_columns", "Y轴列(逗号分隔)", ParamType::String, QString());
	addParam("x_label", "X轴标签", ParamType::String, QString());
	addParam("y_label", "Y轴标签", ParamType::String, QString());
	addParam("show_legend", "显示图例", ParamType::Bool, true);
	addParam("fill_opacity", "填充透明度", ParamType::DoubleSlider, 0.3,
			 { {"min", 0.0}, {"max", 1.0} });
}

void AreaChart::process()
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
	QStringList yCols = param("y_columns").toString().split(",", Qt::SkipEmptyParts);
	for (auto& s : yCols) s = s.trimmed();

	if (!table->hasColumn(xCol))
	{
		reportError(QString("X轴列[%1]不存在").arg(xCol)); return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	double fillOpacity = param("fill_opacity").toDouble();

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
	QVector<double> xData;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		bool ok;
		double v = table->value(r, xIdx).toDouble(&ok);
		xData.append(ok ? v : static_cast<double>(r));
	}

	auto palette = ChartHelper::defaultPalette();

	for (int g = 0; g < yCols.size(); ++g)
	{
		if (!table->hasColumn(yCols[g])) continue;
		int yIdx = table->columnIndex(yCols[g]);
		QVector<double> yData;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			bool ok;
			double v = table->value(r, yIdx).toDouble(&ok);
			yData.append(ok ? v : 0.0);
		}

		QCPGraph* graph = plot.addGraph();
		graph->setData(xData, yData);
		graph->setName(yCols[g]);
		QColor lineColor = palette[g % palette.size()];
		graph->setPen(QPen(lineColor, 2));
		QColor fillColor = lineColor;
		fillColor.setAlphaF(fillOpacity);
		graph->setBrush(QBrush(fillColor));
	}

	plot.xAxis->setLabel(param("x_label").toString());
	plot.yAxis->setLabel(param("y_label").toString());
	if (param("show_legend").toBool()) plot.legend->setVisible(true);
	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
