// nodes/stylize/EmbossEffect.cpp
#include "EmbossEffect.h"
#include <opencv2/imgproc.hpp>

void EmbossEffect::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
	addParam("direction", "方向", ParamType::Combo, QString("左上"),
		{{"options",QStringList{"左上","右上","左下","右下"}}});
	addParam("strength", "强度", ParamType::IntSlider, 128, {{"min",32},{"max",255}});
}

void EmbossEffect::process()
{
	auto d = getInput("input_image");
	if (!d||d->isNull()) { reportError("输入图像为空"); return; }
	cv::Mat src = d->toImage(), gray, dst;
	if (src.channels()>1) cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	else gray = src.clone();
	QString dir = param("direction").toString();
	int s = param("strength").toInt();
	cv::Mat kernel = (cv::Mat_<float>(3,3) <<
		(dir=="左上"?-2:dir=="右上"?0:dir=="左下"?0:2), (dir=="左上"?-1:dir=="右上"?-1:dir=="左下"?1:1), 0,
		(dir=="左上"?-1:dir=="右上"?1:dir=="左下"?-1:1), 0, (dir=="左上"?1:dir=="右上"?1:dir=="左下"?-1:-1),
		0, (dir=="左上"?1:dir=="右上"?1:dir=="左下"?-1:-1), (dir=="左上"?2:dir=="右上"?0:dir=="左下"?0:-2));
	cv::filter2D(gray, dst, CV_32F, kernel);
	dst = dst + s;
	dst.convertTo(dst, CV_8U);
	cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);
	setOutput("output_image", NodeData::createImage(dst));
}
