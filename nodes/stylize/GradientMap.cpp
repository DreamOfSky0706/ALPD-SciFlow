// nodes/stylize/GradientMap.cpp
#include "GradientMap.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

void GradientMap::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	// 默认黑到白
	QVariantList defaultStops;
	QVariantMap s1; s1["pos"] = 0.0; s1["color"] = QVariantList{ 0, 0, 0, 255 };
	QVariantMap s2; s2["pos"] = 1.0; s2["color"] = QVariantList{ 255, 255, 255, 255 };
	defaultStops << QVariant(s1) << QVariant(s2);
	addParam("color_stops", "渐变色标", ParamType::GradientEditor, defaultStops);
}

void GradientMap::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	cv::Mat gray;
	if (src.channels() >= 3) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else gray = src;

	QVariantList stops = param("color_stops").toList();
	if (stops.size() < 2)
	{
		reportError("至少需要两个色标"); return;
	}

	// 排序色标
	struct Stop
	{
		double pos; QColor color;
	};
	QVector<Stop> sortedStops;
	for (const auto& s : stops)
	{
		QVariantMap m = s.toMap();
		Stop st;
		st.pos = m.value("pos", 0.0).toDouble();
		st.color = Utility::arrayToColor(m.value("color").toList());
		sortedStops.append(st);
	}
	std::sort(sortedStops.begin(), sortedStops.end(),
			  [](const Stop& a, const Stop& b)
			  {
				  return a.pos < b.pos;
			  });

	// 构建256项BGR查找表
	cv::Mat lut(1, 256, CV_8UC3);
	for (int i = 0; i < 256; ++i)
	{
		double t = i / 255.0;

		// 在色标间插值
		QColor c;
		if (t <= sortedStops.first().pos)
		{
			c = sortedStops.first().color;
		}
		else if (t >= sortedStops.last().pos)
		{
			c = sortedStops.last().color;
		}
		else
		{
			for (int j = 0; j < sortedStops.size() - 1; ++j)
			{
				if (t >= sortedStops[j].pos && t <= sortedStops[j + 1].pos)
				{
					double segLen = sortedStops[j + 1].pos - sortedStops[j].pos;
					double localT = (segLen > 0) ? (t - sortedStops[j].pos) / segLen : 0;
					int r = static_cast<int>(sortedStops[j].color.red() * (1 - localT) + sortedStops[j + 1].color.red() * localT);
					int g = static_cast<int>(sortedStops[j].color.green() * (1 - localT) + sortedStops[j + 1].color.green() * localT);
					int b = static_cast<int>(sortedStops[j].color.blue() * (1 - localT) + sortedStops[j + 1].color.blue() * localT);
					c = QColor(r, g, b);
					break;
				}
			}
		}

		lut.at<cv::Vec3b>(0, i) = cv::Vec3b(c.blue(), c.green(), c.red());
	}

	// 转三通道灰度以使用BGR LUT
	cv::Mat gray3;
	cv::cvtColor(gray, gray3, cv::COLOR_GRAY2BGR);

	// 拆通道分别LUT（因为LUT要求类型匹配）
	// 直接用灰度值索引LUT
	cv::Mat dst(gray.size(), CV_8UC3);
	for (int y = 0; y < gray.rows; ++y)
	{
		const uchar* gRow = gray.ptr<uchar>(y);
		uchar* dRow = dst.ptr<uchar>(y);
		for (int x = 0; x < gray.cols; ++x)
		{
			cv::Vec3b color = lut.at<cv::Vec3b>(0, gRow[x]);
			dRow[x * 3] = color[0];
			dRow[x * 3 + 1] = color[1];
			dRow[x * 3 + 2] = color[2];
		}
	}

	setOutput("output_image", NodeData::createImage(dst));
}
