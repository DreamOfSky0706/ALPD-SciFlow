// nodes/stylize/OilPaintEffect.cpp
#include "OilPaintEffect.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

void OilPaintEffect::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("brush_size", "笔触半径", ParamType::IntSlider, 7, { {"min", 1}, {"max", 20} });
	addParam("smooth_level", "颜色级数", ParamType::IntSlider, 20, { {"min", 1}, {"max", 256} });
}

void OilPaintEffect::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	if (src.channels() == 4) cv::cvtColor(src, src, cv::COLOR_BGRA2BGR);
	if (src.channels() == 1) cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);

	int radius = param("brush_size").toInt();
	int levels = param("smooth_level").toInt();

	cv::Mat dst = cv::Mat::zeros(src.size(), src.type());

	for (int y = 0; y < src.rows; ++y)
	{
		for (int x = 0; x < src.cols; ++x)
		{
			QVector<int> intensityCount(levels, 0);
			QVector<int> sumB(levels, 0), sumG(levels, 0), sumR(levels, 0);

			for (int dy = -radius; dy <= radius; ++dy)
			{
				for (int dx = -radius; dx <= radius; ++dx)
				{
					int ny = std::clamp(y + dy, 0, src.rows - 1);
					int nx = std::clamp(x + dx, 0, src.cols - 1);

					const uchar* p = src.ptr<uchar>(ny) + nx * 3;
					int intensity = (static_cast<int>(p[0]) + p[1] + p[2]) / 3;
					int bin = intensity * levels / 256;
					bin = std::clamp(bin, 0, levels - 1);

					intensityCount[bin]++;
					sumB[bin] += p[0];
					sumG[bin] += p[1];
					sumR[bin] += p[2];
				}
			}

			int maxBin = 0;
			int maxCount = 0;
			for (int i = 0; i < levels; ++i)
			{
				if (intensityCount[i] > maxCount)
				{
					maxCount = intensityCount[i];
					maxBin = i;
				}
			}

			uchar* dp = dst.ptr<uchar>(y) + x * 3;
			if (maxCount > 0)
			{
				dp[0] = static_cast<uchar>(sumB[maxBin] / maxCount);
				dp[1] = static_cast<uchar>(sumG[maxBin] / maxCount);
				dp[2] = static_cast<uchar>(sumR[maxBin] / maxCount);
			}
		}
	}

	setOutput("output_image", NodeData::createImage(dst));
}
