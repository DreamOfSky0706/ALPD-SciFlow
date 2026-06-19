// nodes/image_io/GradientGenerate.cpp
#include "GradientGenerate.h"
#include "Utility.h"
#include <cmath>

void GradientGenerate::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("size_mode", "尺寸模式", ParamType::Combo, QString("像素"),
			 { {"options", QStringList{"像素", "物理尺寸(mm)+DPI", "物理尺寸(inch)+DPI"}} });
	addParam("width", "宽度", ParamType::Int, 800, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 600, { {"min", 1}, {"max", 20000} });
	addParam("dpi", "DPI", ParamType::Int, 300,
			 { {"min", 72}, {"max", 2400}, {"visible_when", "size_mode!=像素"} });
	addParam("gradient_type", "渐变类型", ParamType::Combo, QString("线性渐变"),
			 { {"options", QStringList{"线性渐变", "径向渐变"}} });
	addParam("color_start", "起始颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("color_end", "结束颜色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("angle", "角度", ParamType::DoubleSlider, 0.0,
			 { {"min", 0.0}, {"max", 360.0}, {"visible_when", "gradient_type==线性渐变"} });
	addParam("center_x", "中心X", ParamType::DoubleSlider, 0.5,
			 { {"min", 0.0}, {"max", 1.0}, {"visible_when", "gradient_type==径向渐变"} });
	addParam("center_y", "中心Y", ParamType::DoubleSlider, 0.5,
			 { {"min", 0.0}, {"max", 1.0}, {"visible_when", "gradient_type==径向渐变"} });
	addParam("radius", "半径", ParamType::DoubleSlider, 1.0,
			 { {"min", 0.1}, {"max", 2.0}, {"visible_when", "gradient_type==径向渐变"} });
}

void GradientGenerate::process()
{
	QString sizeMode = param("size_mode").toString();
	int w = param("width").toInt();
	int h = param("height").toInt();
	int dpi = param("dpi").toInt();

	if (sizeMode == "物理尺寸(mm)+DPI")
	{
		w = safeRound((w / 25.4 * dpi));
		h = safeRound((h / 25.4 * dpi));
	}
	else if (sizeMode == "物理尺寸(inch)+DPI")
	{
		w = safeRound((w * dpi));
		h = safeRound((h * dpi));
	}

	if (w < 1 || w > 20000 || h < 1 || h > 20000)
	{
		reportError(QString("图像尺寸超出范围：%1x%2").arg(w).arg(h));
		return;
	}

	QColor cStart = Utility::arrayToColor(param("color_start").toList());
	QColor cEnd = Utility::arrayToColor(param("color_end").toList());
	QString gradType = param("gradient_type").toString();

	bool useAlpha = (cStart.alpha() < 255 || cEnd.alpha() < 255);
	int channels = useAlpha ? 4 : 3;
	int cvType = useAlpha ? CV_8UC4 : CV_8UC3;

	cv::Mat img(h, w, cvType);

	if (gradType == "线性渐变")
	{
		double angleDeg = param("angle").toDouble();
		double angleRad = angleDeg * CV_PI / 180.0;
		double dx = std::cos(angleRad);
		double dy = std::sin(angleRad);

		// 计算投影范围
		double minProj = 0;
		double maxProj = 0;
		double corners[4];
		corners[0] = 0;
		corners[1] = (w - 1) * dx;
		corners[2] = (h - 1) * dy;
		corners[3] = (w - 1) * dx + (h - 1) * dy;
		minProj = *std::min_element(corners, corners + 4);
		maxProj = *std::max_element(corners, corners + 4);
		double range = maxProj - minProj;
		if (range < 1e-6)
		{
			range = 1.0;
		}

		for (int y = 0; y < h; ++y)
		{
			uchar* row = img.ptr<uchar>(y);
			for (int x = 0; x < w; ++x)
			{
				double proj = x * dx + y * dy;
				double t = (proj - minProj) / range;
				t = std::max(0.0, std::min(1.0, t));

				int b = static_cast<int>(cStart.blue() * (1.0 - t) + cEnd.blue() * t);
				int g = static_cast<int>(cStart.green() * (1.0 - t) + cEnd.green() * t);
				int r = static_cast<int>(cStart.red() * (1.0 - t) + cEnd.red() * t);

				int idx = x * channels;
				row[idx] = static_cast<uchar>(b);
				row[idx + 1] = static_cast<uchar>(g);
				row[idx + 2] = static_cast<uchar>(r);
				if (useAlpha)
				{
					int a = static_cast<int>(cStart.alpha() * (1.0 - t) + cEnd.alpha() * t);
					row[idx + 3] = static_cast<uchar>(a);
				}
			}
		}
	}
	else
	{
		// 径向渐变
		double cx = param("center_x").toDouble() * (w - 1);
		double cy = param("center_y").toDouble() * (h - 1);
		double diag = std::sqrt(w * w + h * h);
		double rad = param("radius").toDouble() * diag * 0.5;
		if (rad < 1.0)
		{
			rad = 1.0;
		}

		for (int y = 0; y < h; ++y)
		{
			uchar* row = img.ptr<uchar>(y);
			for (int x = 0; x < w; ++x)
			{
				double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				double t = dist / rad;
				t = std::max(0.0, std::min(1.0, t));

				int b = static_cast<int>(cStart.blue() * (1.0 - t) + cEnd.blue() * t);
				int g = static_cast<int>(cStart.green() * (1.0 - t) + cEnd.green() * t);
				int r = static_cast<int>(cStart.red() * (1.0 - t) + cEnd.red() * t);

				int idx = x * channels;
				row[idx] = static_cast<uchar>(b);
				row[idx + 1] = static_cast<uchar>(g);
				row[idx + 2] = static_cast<uchar>(r);
				if (useAlpha)
				{
					int a = static_cast<int>(cStart.alpha() * (1.0 - t) + cEnd.alpha() * t);
					row[idx + 3] = static_cast<uchar>(a);
				}
			}
		}
	}

	setOutput("output_image", NodeData::createImage(img));
}
