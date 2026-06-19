// nodes/chart/Histogram.cpp
#include "Histogram.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"
#include <opencv2/imgproc.hpp>

void Histogram::defineNode()
{
	addInputPort("input_data", { DataType::DataTable, DataType::Image });
	addOutputPort("output_image", DataType::Image);

	addParam("chart_width", "图表宽度", ParamType::Int, 800, { {"min", 200}, {"max", 4000} });
	addParam("chart_height", "图表高度", ParamType::Int, 600, { {"min", 200}, {"max", 4000} });
	addParam("title", "标题", ParamType::String, QString("直方图"));
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("bin_count", "分箱数", ParamType::IntSlider, 20, { {"min", 2}, {"max", 200} });
	addParam("bar_color", "柱颜色", ParamType::Color, QVariantList{ 78, 121, 167, 255 });
	// Image模式专用
	addParam("channel", "通道(图像模式)", ParamType::Combo, QString("全部通道叠加"),
			 { {"options", QStringList{"灰度/亮度", "R", "G", "B", "全部通道叠加"}} });
	// DataTable模式专用
	addParam("data_column", "数据列(表格模式)", ParamType::String, QString());
}

void Histogram::onInputConnected(const QString& portName, DataType actualType)
{
	Q_UNUSED(portName);
	Q_UNUSED(actualType);
}

void Histogram::process()
{
	auto inputData = getInput("input_data");
	if (!inputData || inputData->isNull())
	{
		reportError("输入数据为空"); return;
	}

	int w = param("chart_width").toInt();
	int h = param("chart_height").toInt();
	int bins = param("bin_count").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	QColor barColor = Utility::arrayToColor(param("bar_color").toList());

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

	if (inputData->dataType() == DataType::Image)
	{
		cv::Mat img = inputData->toImage();
		cv::Mat gray;
		if (img.channels() >= 3)
		{
			cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
		}
		else
		{
			gray = img;
		}

		int histSize = bins;
		float range[] = { 0, 256 };
		const float* histRange = { range };
		cv::Mat hist;
		cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

		QCPBars* bars = new QCPBars(plot.xAxis, plot.yAxis);
		bars->setPen(QPen(barColor.darker(120)));
		bars->setBrush(barColor);
		bars->setWidth(256.0 / histSize * 0.9);

		QVector<double> keys, values;
		for (int i = 0; i < histSize; ++i)
		{
			keys.append(i * 256.0 / histSize + 128.0 / histSize);
			values.append(hist.at<float>(i));
		}
		bars->setData(keys, values);
		plot.xAxis->setRange(0, 256);
	}
	else
	{
		auto table = inputData->toDataTable();
		if (!table)
		{
			reportError("无法获取DataTable"); return;
		}

		QString colName = param("data_column").toString();
		if (!table->hasColumn(colName))
		{
			reportError(QString("列[%1]不存在").arg(colName)); return;
		}

		int colIdx = table->columnIndex(colName);
		QVector<double> rawData;
		for (int r = 0; r < table->rowCount(); ++r)
		{
			bool ok;
			double v = table->value(r, colIdx).toDouble(&ok);
			if (ok) rawData.append(v);
		}

		if (rawData.isEmpty())
		{
			reportError("无有效数值数据"); return;
		}

		double minV = *std::min_element(rawData.begin(), rawData.end());
		double maxV = *std::max_element(rawData.begin(), rawData.end());
		double binWidth = (maxV - minV) / bins;
		if (binWidth < 1e-12) binWidth = 1.0;

		QVector<double> counts(bins, 0);
		for (double v : rawData)
		{
			int idx = static_cast<int>((v - minV) / binWidth);
			if (idx >= bins) idx = bins - 1;
			if (idx < 0) idx = 0;
			counts[idx]++;
		}

		QCPBars* bars = new QCPBars(plot.xAxis, plot.yAxis);
		bars->setPen(QPen(barColor.darker(120)));
		bars->setBrush(barColor);
		bars->setWidth(binWidth * 0.9);

		QVector<double> keys;
		for (int i = 0; i < bins; ++i)
		{
			keys.append(minV + (i + 0.5) * binWidth);
		}
		bars->setData(keys, counts);
	}

	plot.rescaleAxes();
	plot.replot();

	QPixmap pixmap = plot.toPixmap(w, h);
	cv::Mat dst = Utility::qImageToMat(pixmap.toImage());
	setOutput("output_image", NodeData::createImage(dst));
}
