// nodes/chart/LineChart.cpp
#include "LineChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"

void LineChart::defineNode()
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
}

void LineChart::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull())
	{
		reportError("输入数据为空");
		return;
	}
	auto table = inputData->toDataTable();
	if (!table)
	{
		reportError("无法获取DataTable"); return;
	}

	QString xCol = param("x_column").toString();
	QString yColsStr = param("y_columns").toString();
	QStringList yCols = yColsStr.split(",", Qt::SkipEmptyParts);
	for (auto& s : yCols) s = s.trimmed();

	if (!table->hasColumn(xCol))
	{
		reportError(QString("X轴列[%1]不存在").arg(xCol)); return;
	}
	for (const auto& yc : yCols)
	{
		if (!table->hasColumn(yc))
		{
			reportError(QString("Y轴列[%1]不存在").arg(yc)); return;
		}
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
		QCPTextElement* titleElem = new QCPTextElement(&plot, title);
		titleElem->setFont(QFont(QString(), param("title_font_size").toInt(), QFont::Bold));
		plot.plotLayout()->addElement(0, 0, titleElem);
	}

	// X轴数据
	int xIdx = table->columnIndex(xCol);
	QVector<double> xData;
	QVector<QString> xLabels;
	bool xIsNumeric = true;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		bool ok;
		double v = table->value(r, xIdx).toDouble(&ok);
		if (ok)
		{
			xData.append(v);
		}
		else
		{
			xIsNumeric = false;
			xData.append(r);
		}
		xLabels.append(table->value(r, xIdx).toString());
	}

	if (!xIsNumeric)
	{
		QSharedPointer<QCPAxisTickerText> ticker(new QCPAxisTickerText);
		for (int i = 0; i < xLabels.size(); ++i)
		{
			ticker->addTick(i, xLabels[i]);
		}
		plot.xAxis->setTicker(ticker);
	}

	auto palette = ChartHelper::defaultPalette();

	for (int g = 0; g < yCols.size(); ++g)
	{
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
		QPen pen(palette[g % palette.size()]);
		pen.setWidth(2);
		graph->setPen(pen);
	}

	plot.xAxis->setLabel(param("x_label").toString());
	plot.yAxis->setLabel(param("y_label").toString());

	if (param("show_legend").toBool() && yCols.size() > 1)
	{
		plot.legend->setVisible(true);
	}

	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	QImage qimg = pixmap.toImage();
	cv::Mat dst = Utility::qImageToMat(qimg);
	setOutput("output_image", NodeData::createImage(dst));
}
