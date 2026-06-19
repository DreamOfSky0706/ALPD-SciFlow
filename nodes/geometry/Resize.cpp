// nodes/geometry/Resize.cpp
#include "Resize.h"
#include <opencv2/imgproc.hpp>

void Resize::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("mode", "模式", ParamType::Combo, QString("按比例缩放"),
			 { {"options", QStringList{"指定尺寸", "按比例缩放"}} });
	addParam("target_width", "目标宽度", ParamType::Int, 800,
			 { {"min", 1}, {"max", 20000}, {"visible_when", "mode==指定尺寸"} });
	addParam("target_height", "目标高度", ParamType::Int, 600,
			 { {"min", 1}, {"max", 20000}, {"visible_when", "mode==指定尺寸"} });
	addParam("scale_factor", "缩放比例", ParamType::DoubleSlider, 1.0,
			 { {"min", 0.01}, {"max", 20.0}, {"visible_when", "mode==按比例缩放"} });
	addParam("keep_aspect_ratio", "保持宽高比", ParamType::Bool, true,
			 { {"visible_when", "mode==指定尺寸"} });
	addParam("interpolation", "插值方法", ParamType::Combo, QString("双线性"),
			 { {"options", QStringList{"最近邻", "双线性", "双三次", "Lanczos"}} });
}

void Resize::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QString mode = param("mode").toString();

	int dstW, dstH;

	if (mode == "按比例缩放")
	{
		double factor = param("scale_factor").toDouble();
		dstW = safeRound((src.cols * factor));
		dstH = safeRound((src.rows * factor));
	}
	else
	{
		dstW = param("target_width").toInt();
		dstH = param("target_height").toInt();

		if (param("keep_aspect_ratio").toBool())
		{
			double ratioW = static_cast<double>(dstW) / src.cols;
			double ratioH = static_cast<double>(dstH) / src.rows;
			double ratio = std::min(ratioW, ratioH);
			dstW = safeRound((src.cols * ratio));
			dstH = safeRound((src.rows * ratio));
		}
	}

	if (dstW < 1 || dstH < 1)
	{
		reportError("计算后的目标尺寸无效");
		return;
	}

	// 映射插值方法
	QString interpName = param("interpolation").toString();
	int interp = cv::INTER_LINEAR;
	if (interpName == "最近邻") interp = cv::INTER_NEAREST;
	else if (interpName == "双三次") interp = cv::INTER_CUBIC;
	else if (interpName == "Lanczos") interp = cv::INTER_LANCZOS4;

	cv::Mat dst;
	cv::resize(src, dst, cv::Size(dstW, dstH), 0, 0, interp);
	setOutput("output_image", NodeData::createImage(dst));
}
