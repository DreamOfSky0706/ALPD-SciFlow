// nodes/composite/Blend.cpp
#include "Blend.h"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <algorithm>

void Blend::defineNode()
{
	addInputPort("input_base", DataType::Image);
	addInputPort("input_top", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("blend_mode", "混合模式", ParamType::Combo, QString("正常(Normal)"),
			 { {"options", QStringList{
				   "正常(Normal)", "正片叠底(Multiply)", "滤色(Screen)",
				   "叠加(Overlay)", "柔光(SoftLight)", "强光(HardLight)",
				   "差值(Difference)", "排除(Exclusion)",
				   "变暗(Darken)", "变亮(Lighten)",
				   "颜色加深(ColorBurn)", "颜色减淡(ColorDodge)",
				   "线性加深(LinearBurn)", "线性减淡(LinearDodge)",
				   "亮光(VividLight)"}} });
	addParam("opacity", "不透明度", ParamType::DoubleSlider, 1.0,
			 { {"min", 0.0}, {"max", 1.0} });
}

void Blend::process()
{
	auto baseData = getInput("input_base");
	auto topData = getInput("input_top");

	if (!baseData || baseData->isNull())
	{
		reportError("底层图像为空");
		return;
	}
	if (!topData || topData->isNull())
	{
		reportError("顶层图像为空");
		return;
	}

	cv::Mat base = baseData->toImage();
	cv::Mat top = topData->toImage();

	// 统一尺寸
	if (top.size() != base.size())
	{
		cv::resize(top, top, base.size());
	}

	// 统一通道数为三通道
	if (base.channels() == 1)
	{
		cv::cvtColor(base, base, cv::COLOR_GRAY2BGR);
	}
	else if (base.channels() == 4)
	{
		cv::cvtColor(base, base, cv::COLOR_BGRA2BGR);
	}

	if (top.channels() == 1)
	{
		cv::cvtColor(top, top, cv::COLOR_GRAY2BGR);
	}
	else if (top.channels() == 4)
	{
		cv::cvtColor(top, top, cv::COLOR_BGRA2BGR);
	}

	QString mode = param("blend_mode").toString();
	double opacity = param("opacity").toDouble();

	cv::Mat dst = base.clone();

	for (int y = 0; y < base.rows; ++y)
	{
		const uchar* baseRow = base.ptr<uchar>(y);
		const uchar* topRow = top.ptr<uchar>(y);
		uchar* dstRow = dst.ptr<uchar>(y);

		for (int x = 0; x < base.cols; ++x)
		{
			int idx = x * 3;
			for (int c = 0; c < 3; ++c)
			{
				double a = baseRow[idx + c] / 255.0;
				double b = topRow[idx + c] / 255.0;
				double blended = blendPixel(a, b, mode);
				// 混合不透明度
				double result = a * (1.0 - opacity) + blended * opacity;
				result = std::clamp(result, 0.0, 1.0);
				dstRow[idx + c] = static_cast<uchar>(safeRound((result * 255.0)));
			}
		}
	}

	setOutput("output_image", NodeData::createImage(dst));
}

double Blend::blendPixel(double a, double b, const QString& mode) const
{
	if (mode.startsWith("正常"))
		return b;
	if (mode.startsWith("正片叠底"))
		return a * b;
	if (mode.startsWith("滤色"))
		return 1.0 - (1.0 - a) * (1.0 - b);
	if (mode.startsWith("叠加"))
		return (a < 0.5) ? 2.0 * a * b : 1.0 - 2.0 * (1.0 - a) * (1.0 - b);
	if (mode.startsWith("柔光"))
		return (b < 0.5) ? a - (1.0 - 2.0 * b) * a * (1.0 - a)
		: a + (2.0 * b - 1.0) * (std::sqrt(a) - a);
	if (mode.startsWith("强光"))
		return (b < 0.5) ? 2.0 * a * b : 1.0 - 2.0 * (1.0 - a) * (1.0 - b);
	if (mode.startsWith("差值"))
		return std::abs(a - b);
	if (mode.startsWith("排除"))
		return a + b - 2.0 * a * b;
	if (mode.startsWith("变暗"))
		return std::min(a, b);
	if (mode.startsWith("变亮"))
		return std::max(a, b);
	if (mode.startsWith("颜色加深"))
		return (b < 1e-6) ? 0.0 : std::max(0.0, 1.0 - (1.0 - a) / b);
	if (mode.startsWith("颜色减淡"))
		return (b > 1.0 - 1e-6) ? 1.0 : std::min(1.0, a / (1.0 - b));
	if (mode.startsWith("线性加深"))
		return std::max(0.0, a + b - 1.0);
	if (mode.startsWith("线性减淡"))
		return std::min(1.0, a + b);
	if (mode.startsWith("亮光"))
	{
		if (b < 0.5)
		{
			double burn = (2.0 * b < 1e-6) ? 0.0 : std::max(0.0, 1.0 - (1.0 - a) / (2.0 * b));
			return burn;
		}
		else
		{
			double dodge = ((2.0 * b - 1.0) > 1.0 - 1e-6) ? 1.0 : std::min(1.0, a / (1.0 - (2.0 * b - 1.0)));
			return dodge;
		}
	}
	return b;
}
