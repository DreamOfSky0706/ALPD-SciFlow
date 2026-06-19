// nodes/composite/ImageStack.cpp
#include "ImageStack.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void ImageStack::defineNode()
{
	addInputPort("input_a", DataType::Image);
	addInputPort("input_b", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("direction", "拼接方向", ParamType::Combo, QString("水平（左右）"),
			 { {"options", QStringList{"水平（左右）", "垂直（上下）"}} });
	addParam("alignment", "对齐方式", ParamType::Combo, QString("居中对齐"),
			 { {"options", QStringList{"顶部对齐", "居中对齐", "底部对齐",
									  "左对齐", "居中对齐", "右对齐"}} });
	addParam("gap", "间距", ParamType::IntSlider, 0, { {"min", 0}, {"max", 200} });
	addParam("gap_color", "间距颜色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
}

void ImageStack::process()
{
	auto dA = getInput("input_a");
	auto dB = getInput("input_b");

	if (!dA || dA->isNull() || !dB || dB->isNull())
	{
		reportError("两个输入图像不能为空");
		return;
	}

	cv::Mat a = dA->toImage();
	cv::Mat b = dB->toImage();

	// 统一通道数
	int maxCh = std::max(a.channels(), b.channels());
	if (a.channels() == 1 && maxCh >= 3)
	{
		cv::cvtColor(a, a, cv::COLOR_GRAY2BGR);
	}
	if (b.channels() == 1 && maxCh >= 3)
	{
		cv::cvtColor(b, b, cv::COLOR_GRAY2BGR);
	}
	if (a.channels() == 3 && maxCh == 4)
	{
		cv::cvtColor(a, a, cv::COLOR_BGR2BGRA);
	}
	if (b.channels() == 3 && maxCh == 4)
	{
		cv::cvtColor(b, b, cv::COLOR_BGR2BGRA);
	}

	QString direction = param("direction").toString();
	QString alignment = param("alignment").toString();
	int gap = param("gap").toInt();
	QColor gapColor = Utility::arrayToColor(param("gap_color").toList());
	cv::Scalar gapScalar = Utility::colorToScalar(gapColor, a.channels() == 4);

	int dstW, dstH;
	bool horizontal = direction.contains("水平");

	if (horizontal)
	{
		dstW = a.cols + gap + b.cols;
		dstH = std::max(a.rows, b.rows);
	}
	else
	{
		dstW = std::max(a.cols, b.cols);
		dstH = a.rows + gap + b.rows;
	}

	cv::Mat dst(dstH, dstW, a.type(), gapScalar);

	// 计算各图的放置坐标
	int axOffset = 0, ayOffset = 0;
	int bxOffset = 0, byOffset = 0;

	if (horizontal)
	{
		bxOffset = a.cols + gap;

		// 垂直对齐
		if (alignment.contains("顶部") || alignment.contains("左"))
		{
			ayOffset = 0;
			byOffset = 0;
		}
		else if (alignment.contains("底部") || alignment.contains("右"))
		{
			ayOffset = dstH - a.rows;
			byOffset = dstH - b.rows;
		}
		else
		{
			ayOffset = (dstH - a.rows) / 2;
			byOffset = (dstH - b.rows) / 2;
		}
	}
	else
	{
		byOffset = a.rows + gap;

		// 水平对齐
		if (alignment.contains("左") || alignment.contains("顶部"))
		{
			axOffset = 0;
			bxOffset = 0;
		}
		else if (alignment.contains("右") || alignment.contains("底部"))
		{
			axOffset = dstW - a.cols;
			bxOffset = dstW - b.cols;
		}
		else
		{
			axOffset = (dstW - a.cols) / 2;
			bxOffset = (dstW - b.cols) / 2;
		}
	}

	a.copyTo(dst(cv::Rect(axOffset, ayOffset, a.cols, a.rows)));
	b.copyTo(dst(cv::Rect(bxOffset, byOffset, b.cols, b.rows)));

	setOutput("output_image", NodeData::createImage(dst));
}
