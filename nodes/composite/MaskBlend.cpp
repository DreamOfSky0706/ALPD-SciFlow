// nodes/composite/MaskBlend.cpp
#include "MaskBlend.h"
#include <opencv2/imgproc.hpp>

void MaskBlend::defineNode()
{
	addInputPort("input_a", DataType::Image);
	addInputPort("input_b", DataType::Image);
	addInputPort("input_mask", DataType::Image);
	addOutputPort("output_image", DataType::Image);
}

void MaskBlend::process()
{
	auto dA = getInput("input_a");
	auto dB = getInput("input_b");
	auto dM = getInput("input_mask");

	if (!dA || dA->isNull() || !dB || dB->isNull() || !dM || dM->isNull())
	{
		reportError("三个输入端口均不能为空");
		return;
	}

	cv::Mat a = dA->toImage();
	cv::Mat b = dB->toImage();
	cv::Mat mask = dM->toImage();

	// 蒙版转灰度
	if (mask.channels() != 1)
	{
		cv::cvtColor(mask, mask, cv::COLOR_BGR2GRAY);
		reportWarning("蒙版非单通道，已自动转灰度");
	}

	// 统一尺寸到a
	if (b.size() != a.size())
	{
		cv::resize(b, b, a.size());
		reportWarning("图像B尺寸与A不一致，已自动缩放");
	}
	if (mask.size() != a.size())
	{
		cv::resize(mask, mask, a.size());
		reportWarning("蒙版尺寸与A不一致，已自动缩放");
	}

	// 统一通道数
	if (a.channels() == 1)
	{
		cv::cvtColor(a, a, cv::COLOR_GRAY2BGR);
	}
	else if (a.channels() == 4)
	{
		cv::cvtColor(a, a, cv::COLOR_BGRA2BGR);
	}

	if (b.channels() == 1)
	{
		cv::cvtColor(b, b, cv::COLOR_GRAY2BGR);
	}
	else if (b.channels() == 4)
	{
		cv::cvtColor(b, b, cv::COLOR_BGRA2BGR);
	}

	// output = a * (1 - mask/255) + b * (mask/255)
	cv::Mat maskF;
	mask.convertTo(maskF, CV_32F, 1.0 / 255.0);

	cv::Mat aF, bF;
	a.convertTo(aF, CV_32F);
	b.convertTo(bF, CV_32F);

	// 将mask扩展到三通道
	cv::Mat mask3;
	cv::merge(std::vector<cv::Mat>{maskF, maskF, maskF}, mask3);

	cv::Mat resultF = aF.mul(cv::Scalar(1.0, 1.0, 1.0) - mask3) + bF.mul(mask3);

	cv::Mat dst;
	resultF.convertTo(dst, CV_8U);
	setOutput("output_image", NodeData::createImage(dst));
}
