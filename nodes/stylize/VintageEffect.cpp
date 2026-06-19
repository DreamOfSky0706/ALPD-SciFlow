// nodes/stylize/VintageEffect.cpp
#include "VintageEffect.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void VintageEffect::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("tone_color", "色调颜色", ParamType::Color, QVariantList{ 210, 180, 120, 255 });
	addParam("tone_strength", "色调强度", ParamType::DoubleSlider, 0.4,
			 { {"min", 0.0}, {"max", 1.0} });
	addParam("vignette_strength", "暗角强度", ParamType::DoubleSlider, 0.6,
			 { {"min", 0.0}, {"max", 1.0} });
	addParam("noise_amount", "噪点量", ParamType::DoubleSlider, 0.1,
			 { {"min", 0.0}, {"max", 1.0} });
	addParam("desaturation", "去色程度", ParamType::DoubleSlider, 0.3,
			 { {"min", 0.0}, {"max", 1.0} });
}

void VintageEffect::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	cv::Mat src = inputData->toImage();
	if (src.channels() == 1) cv::cvtColor(src, src, cv::COLOR_GRAY2BGR);
	if (src.channels() == 4) cv::cvtColor(src, src, cv::COLOR_BGRA2BGR);

	double desat = param("desaturation").toDouble();
	double toneStr = param("tone_strength").toDouble();
	QColor toneColor = Utility::arrayToColor(param("tone_color").toList());
	double vigStr = param("vignette_strength").toDouble();
	double noiseAmt = param("noise_amount").toDouble();

	cv::Mat result;
	src.convertTo(result, CV_32F, 1.0 / 255.0);

	// 降低饱和度
	cv::Mat gray32;
	cv::cvtColor(result, gray32, cv::COLOR_BGR2GRAY);
	cv::Mat gray3;
	cv::cvtColor(gray32, gray3, cv::COLOR_GRAY2BGR);
	result = result * (1.0 - desat) + gray3 * desat;

	// 叠加色调
	cv::Scalar tone(toneColor.blue() / 255.0, toneColor.green() / 255.0, toneColor.red() / 255.0);
	cv::Mat toneMat(result.size(), result.type(), tone);
	result = result * (1.0 - toneStr) + toneMat * toneStr;

	// 暗角
	if (vigStr > 0.01)
	{
		double cx = result.cols / 2.0;
		double cy = result.rows / 2.0;
		double maxDist = std::sqrt(cx * cx + cy * cy);

		for (int y = 0; y < result.rows; ++y)
		{
			float* row = result.ptr<float>(y);
			for (int x = 0; x < result.cols; ++x)
			{
				double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
				double factor = 1.0 - vigStr * (dist / maxDist);
				factor = std::max(0.0, factor);
				row[x * 3] *= static_cast<float>(factor);
				row[x * 3 + 1] *= static_cast<float>(factor);
				row[x * 3 + 2] *= static_cast<float>(factor);
			}
		}
	}

	// 噪点
	if (noiseAmt > 0.01)
	{
		cv::Mat noise(result.size(), CV_32FC3);
		cv::randn(noise, 0, noiseAmt * 0.15);
		result += noise;
	}

	// clamp and convert
	cv::threshold(result, result, 1.0, 1.0, cv::THRESH_TRUNC);
	cv::threshold(result, result, 0.0, 0.0, cv::THRESH_TOZERO);

	cv::Mat dst;
	result.convertTo(dst, CV_8U, 255.0);
	setOutput("output_image", NodeData::createImage(dst));
}
