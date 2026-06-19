// nodes/color/ColorBalance.cpp
#include "ColorBalance.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

void ColorBalance::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("shadow_r", "阴影R偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("shadow_g", "阴影G偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("shadow_b", "阴影B偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("midtone_r", "中间调R偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("midtone_g", "中间调G偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("midtone_b", "中间调B偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("highlight_r", "高光R偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("highlight_g", "高光G偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
	addParam("highlight_b", "高光B偏移", ParamType::IntSlider, 0, { {"min", -50}, {"max", 50} });
}

void ColorBalance::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	if (src.channels() < 3)
	{
		reportError("色彩平衡需要彩色图像输入");
		return;
	}

	int shR = param("shadow_r").toInt();
	int shG = param("shadow_g").toInt();
	int shB = param("shadow_b").toInt();
	int midR = param("midtone_r").toInt();
	int midG = param("midtone_g").toInt();
	int midB = param("midtone_b").toInt();
	int hiR = param("highlight_r").toInt();
	int hiG = param("highlight_g").toInt();
	int hiB = param("highlight_b").toInt();

	cv::Mat dst = src.clone();
	int channels = dst.channels();

	for (int y = 0; y < dst.rows; ++y)
	{
		uchar* row = dst.ptr<uchar>(y);
		for (int x = 0; x < dst.cols; ++x)
		{
			int idx = x * channels;
			int b = row[idx];
			int g = row[idx + 1];
			int r = row[idx + 2];

			// 亮度作为区域权重依据
			double lum = (r + g + b) / 3.0;

			// 平滑权重：阴影权重在低亮度高，高光权重在高亮度高
			double shadowW = std::max(0.0, 1.0 - lum / 85.0);
			double highlightW = std::max(0.0, (lum - 170.0) / 85.0);
			double midtoneW = 1.0 - shadowW - highlightW;
			midtoneW = std::max(0.0, midtoneW);

			double dR = shadowW * shR + midtoneW * midR + highlightW * hiR;
			double dG = shadowW * shG + midtoneW * midG + highlightW * hiG;
			double dB = shadowW * shB + midtoneW * midB + highlightW * hiB;

			row[idx] = static_cast<uchar>(std::clamp(b + static_cast<int>(dB), 0, 255));
			row[idx + 1] = static_cast<uchar>(std::clamp(g + static_cast<int>(dG), 0, 255));
			row[idx + 2] = static_cast<uchar>(std::clamp(r + static_cast<int>(dR), 0, 255));
		}
	}

	setOutput("output_image", NodeData::createImage(dst));
}
