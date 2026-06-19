// nodes/utility/ImageHistogramStats.cpp
#include "ImageHistogramStats.h"
#include "DataTable.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

void ImageHistogramStats::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_data", DataType::DataTable);
}

void ImageHistogramStats::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat img = inputData->toImage();
	std::vector<cv::Mat> channels;
	cv::split(img, channels);

	QStringList chNames = { "B", "G", "R", "A" };
	if (img.channels() == 1) chNames = { "Gray" };

	auto table = std::make_shared<DataTable>();
	table->addColumn("通道");
	table->addColumn("均值");
	table->addColumn("标准差");
	table->addColumn("最小值");
	table->addColumn("最大值");

	for (int c = 0; c < img.channels() && c < 4; ++c)
	{
		cv::Scalar mean, stddev;
		cv::meanStdDev(channels[c], mean, stddev);
		double minV, maxV;
		cv::minMaxLoc(channels[c], &minV, &maxV);

		table->addRow({ chNames[c], mean[0], stddev[0], minV, maxV });
	}

	setOutput("output_data", NodeData::createDataTable(table));
}
