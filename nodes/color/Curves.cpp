// nodes/color/Curves.cpp
#include "Curves.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

void Curves::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("channel", "通道", ParamType::Combo, QString("RGB"),
			 { {"options", QStringList{"RGB", "R", "G", "B"}} });

	// 控制点列表，每个元素是[x, y]，默认两端点
	QVariantList defaultPoints;
	defaultPoints << QVariant(QVariantList{ 0, 0 });
	defaultPoints << QVariant(QVariantList{ 255, 255 });
	addParam("control_points", "控制点", ParamType::CurveEditor, defaultPoints);
}

void Curves::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QString channel = param("channel").toString();
	QVariantList points = param("control_points").toList();

	if (points.size() < 2)
	{
		reportError("至少需要两个控制点");
		return;
	}

	// 构建查找表
	uchar lutData[256];
	buildLUT(points, lutData);
	cv::Mat lutMat(1, 256, CV_8UC1, lutData);

	if (src.channels() == 1)
	{
		// 灰度图直接应用
		cv::Mat dst;
		cv::LUT(src, lutMat, dst);
		setOutput("output_image", NodeData::createImage(dst));
		return;
	}

	// 多通道处理
	std::vector<cv::Mat> channels;
	cv::split(src, channels);

	// BGR通道索引：0=B, 1=G, 2=R
	if (channel == "RGB" || channel == "R")
	{
		cv::LUT(channels[2], lutMat, channels[2]);
	}
	if (channel == "RGB" || channel == "G")
	{
		cv::LUT(channels[1], lutMat, channels[1]);
	}
	if (channel == "RGB" || channel == "B")
	{
		cv::LUT(channels[0], lutMat, channels[0]);
	}

	cv::Mat dst;
	cv::merge(channels, dst);
	setOutput("output_image", NodeData::createImage(dst));
}

void Curves::buildLUT(const QVariantList& points, uchar* lut) const
{
	struct Point
	{
		double x; double y;
	};
	std::vector<Point> pts;

	for (const auto& p : points)
	{
		QVariantList pair = p.toList();
		if (pair.size() >= 2)
		{
			pts.push_back({ pair[0].toDouble(), pair[1].toDouble() });
		}
	}

	std::sort(pts.begin(), pts.end(), [](const Point& a, const Point& b)
			  {
				  return a.x < b.x;
			  });

	if (pts.size() == 2)
	{
		for (int i = 0; i < 256; ++i)
		{
			double t = 0.0;
			if (pts[1].x != pts[0].x)
			{
				t = (i - pts[0].x) / (pts[1].x - pts[0].x);
			}
			t = std::clamp(t, 0.0, 1.0);
			double val = pts[0].y + t * (pts[1].y - pts[0].y);
			lut[i] = static_cast<uchar>(std::clamp(static_cast<int>(std::round(val)), 0, 255));
		}
		return;
	}

	for (int i = 0; i < 256; ++i)
	{
		double x = static_cast<double>(i);

		if (x <= pts.front().x)
		{
			lut[i] = static_cast<uchar>(std::clamp(static_cast<int>(std::round(pts.front().y)), 0, 255));
			continue;
		}
		if (x >= pts.back().x)
		{
			lut[i] = static_cast<uchar>(std::clamp(static_cast<int>(std::round(pts.back().y)), 0, 255));
			continue;
		}

		for (size_t j = 0; j < pts.size() - 1; ++j)
		{
			if (x >= pts[j].x && x <= pts[j + 1].x)
			{
				double segLen = pts[j + 1].x - pts[j].x;
				double t = (segLen > 0) ? (x - pts[j].x) / segLen : 0.0;
				double val = pts[j].y + t * (pts[j + 1].y - pts[j].y);
				lut[i] = static_cast<uchar>(std::clamp(static_cast<int>(std::round(val)), 0, 255));
				break;
			}
		}
	}
}
