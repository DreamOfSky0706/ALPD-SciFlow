// nodes/chart/Heatmap.cpp
#include "Heatmap.h"
#include "DataTable.h"
#include "Utility.h"
#include "qcustomplot.h"

void Heatmap::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString());
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("row_column", "行标签列", ParamType::String, QString());
	addParam("col_column", "列标签列", ParamType::String, QString());
	addParam("value_column", "数值列", ParamType::String, QString());
	addParam("color_scheme", "配色方案", ParamType::Combo, QString("Viridis"),
			 { {"options", QStringList{"Viridis", "Hot", "Cool", "GrayScale"}} });
	addParam("show_values", "显示数值", ParamType::Bool, false);
}

void Heatmap::process()
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

	QString rowCol = param("row_column").toString();
	QString colCol = param("col_column").toString();
	QString valCol = param("value_column").toString();

	if (!table->hasColumn(rowCol) || !table->hasColumn(colCol) || !table->hasColumn(valCol))
	{
		reportError("指定的列不存在");
		return;
	}

	int rowIdx = table->columnIndex(rowCol);
	int colIdx = table->columnIndex(colCol);
	int valIdx = table->columnIndex(valCol);

	// 收集唯一的行和列标签
	QStringList rowLabels, colLabels;
	for (int r = 0; r < table->rowCount(); ++r)
	{
		QString rl = table->value(r, rowIdx).toString();
		QString cl = table->value(r, colIdx).toString();
		if (!rowLabels.contains(rl)) rowLabels.append(rl);
		if (!colLabels.contains(cl)) colLabels.append(cl);
	}

	int nRows = rowLabels.size();
	int nCols = colLabels.size();

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
		te->setFont(QFont(QString(), 14, QFont::Bold));
		plot.plotLayout()->addElement(0, 0, te);
	}

	QCPColorMap* colorMap = new QCPColorMap(plot.xAxis, plot.yAxis);
	colorMap->data()->setSize(nCols, nRows);
	colorMap->data()->setRange(QCPRange(0, nCols - 1), QCPRange(0, nRows - 1));

	// 初始化为0
	for (int x = 0; x < nCols; ++x)
		for (int y = 0; y < nRows; ++y)
			colorMap->data()->setCell(x, y, 0);

	// 填充数据
	for (int r = 0; r < table->rowCount(); ++r)
	{
		QString rl = table->value(r, rowIdx).toString();
		QString cl = table->value(r, colIdx).toString();
		int ri = rowLabels.indexOf(rl);
		int ci = colLabels.indexOf(cl);
		bool ok;
		double v = table->value(r, valIdx).toDouble(&ok);
		if (ok && ri >= 0 && ci >= 0)
		{
			colorMap->data()->setCell(ci, ri, v);
		}
	}

	// 配色
	QString scheme = param("color_scheme").toString();
	QCPColorGradient gradient;
	if (scheme == "Hot") gradient.loadPreset(QCPColorGradient::gpHot);
	else if (scheme == "Cool") gradient.loadPreset(QCPColorGradient::gpCold);
	else if (scheme == "GrayScale") gradient.loadPreset(QCPColorGradient::gpGrayscale);
	else gradient.loadPreset(QCPColorGradient::gpThermal);
	colorMap->setGradient(gradient);

	colorMap->rescaleDataRange();

	// 标签
	QSharedPointer<QCPAxisTickerText> xTicker(new QCPAxisTickerText);
	for (int i = 0; i < colLabels.size(); ++i)
		xTicker->addTick(i, colLabels[i]);
	plot.xAxis->setTicker(xTicker);

	QSharedPointer<QCPAxisTickerText> yTicker(new QCPAxisTickerText);
	for (int i = 0; i < rowLabels.size(); ++i)
		yTicker->addTick(i, rowLabels[i]);
	plot.yAxis->setTicker(yTicker);

	QCPColorScale* colorScale = new QCPColorScale(&plot);
	plot.plotLayout()->addElement(0, 1, colorScale);
	colorMap->setColorScale(colorScale);

	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
