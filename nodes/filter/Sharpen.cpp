// nodes/filter/Sharpen.cpp
#include "Sharpen.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void Sharpen::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("radius", "模糊半径", ParamType::IntSlider, 3, { {"min", 1}, {"max", 50} });
	addParam("amount", "锐化强度", ParamType::DoubleSlider, 1.5, { {"min", 0.0}, {"max", 5.0} });
	addParam("threshold", "阈值", ParamType::IntSlider, 0, { {"min", 0}, {"max", 255} });
}

void Sharpen::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	int radius = Utility::ensureOdd(param("radius").toInt() * 2 + 1);
	double amount = param("amount").toDouble();
	int threshold = param("threshold").toInt();

	// 高斯模糊
	cv::Mat blurred;
	cv::GaussianBlur(src, blurred, cv::Size(radius, radius), 0);

	// 转浮点做差值
	cv::Mat srcF, blurredF;
	src.convertTo(srcF, CV_32F);
	blurred.convertTo(blurredF, CV_32F);

	cv::Mat diff = srcF - blurredF;

	// 阈值过滤：差值绝对值小于threshold的像素不锐化
	if (threshold > 0)
	{
		cv::Mat absDiff;
		cv::Mat diffGray;

		if (diff.channels() > 1)
		{
			// 取各通道绝对值的最大值作为判断依据
			std::vector<cv::Mat> ch;
			cv::split(diff, ch);
			absDiff = cv::abs(ch[0]);
			for (size_t i = 1; i < ch.size(); ++i)
			{
				absDiff = cv::max(absDiff, cv::abs(ch[i]));
			}
		}
		else
		{
			absDiff = cv::abs(diff);
		}

		// 构造蒙版：绝对值 < threshold的位置置零
		cv::Mat mask;
		cv::compare(absDiff, static_cast<float>(threshold), mask, cv::CMP_LT);

		// 将mask扩展到与diff相同通道数
		if (diff.channels() > 1)
		{
			std::vector<cv::Mat> maskCh(diff.channels(), mask);
			cv::Mat maskMulti;
			cv::merge(maskCh, maskMulti);
			diff.setTo(cv::Scalar::all(0), maskMulti);
		}
		else
		{
			diff.setTo(cv::Scalar(0), mask);
		}
	}

	cv::Mat resultF = srcF + amount * diff;

	cv::Mat dst;
	resultF.convertTo(dst, src.type());
	setOutput("output_image", NodeData::createImage(dst));
}
