// nodes/composite/ImageGridStitch.cpp
#include "ImageGridStitch.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void ImageGridStitch::defineNode()
{
	addInputPort("input_images", DataType::ImageList);
	addOutputPort("output_image", DataType::Image);

	addParam("columns", "列数", ParamType::IntSlider, 2, { {"min", 1}, {"max", 10} });
	addParam("cell_width", "格宽(0=自动)", ParamType::Int, 0, { {"min", 0}, {"max", 5000} });
	addParam("cell_height", "格高(0=自动)", ParamType::Int, 0, { {"min", 0}, {"max", 5000} });
	addParam("gap", "格间距", ParamType::IntSlider, 10, { {"min", 0}, {"max", 100} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("resize_mode", "缩放模式", ParamType::Combo, QString("等比缩放适配"),
			 { {"options", QStringList{"等比缩放适配", "拉伸填满", "不缩放居中"}} });
}

void ImageGridStitch::process()
{
	auto inputData = getInput("input_images");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像列表为空");
		return;
	}

	std::vector<cv::Mat> images = inputData->toImageList();
	if (images.empty())
	{
		reportError("图像列表为空");
		return;
	}

	int cols = param("columns").toInt();
	int rows = static_cast<int>(std::ceil(static_cast<double>(images.size()) / cols));
	int cellW = param("cell_width").toInt();
	int cellH = param("cell_height").toInt();
	int gap = param("gap").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	QString resizeMode = param("resize_mode").toString();

	// 自动计算格子尺寸
	if (cellW <= 0 || cellH <= 0)
	{
		int maxW = 0;
		int maxH = 0;
		for (const auto& img : images)
		{
			maxW = std::max(maxW, img.cols);
			maxH = std::max(maxH, img.rows);
		}
		if (cellW <= 0) cellW = maxW;
		if (cellH <= 0) cellH = maxH;
	}

	int totalW = cols * cellW + (cols - 1) * gap;
	int totalH = rows * cellH + (rows - 1) * gap;

	cv::Scalar bgScalar = Utility::colorToScalar(bgColor, false);
	cv::Mat canvas(totalH, totalW, CV_8UC3, bgScalar);

	for (size_t i = 0; i < images.size(); ++i)
	{
		int r = static_cast<int>(i) / cols;
		int c = static_cast<int>(i) % cols;
		int x0 = c * (cellW + gap);
		int y0 = r * (cellH + gap);

		cv::Mat img = images[i];

		// 统一通道数
		if (img.channels() == 1)
		{
			cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
		}
		else if (img.channels() == 4)
		{
			cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
		}

		cv::Mat cell;
		if (resizeMode == "拉伸填满")
		{
			cv::resize(img, cell, cv::Size(cellW, cellH));
		}
		else if (resizeMode == "不缩放居中")
		{
			cell = cv::Mat(cellH, cellW, CV_8UC3, bgScalar);
			int ox = (cellW - img.cols) / 2;
			int oy = (cellH - img.rows) / 2;
			// 计算有效拷贝区域
			int srcX = std::max(0, -ox);
			int srcY = std::max(0, -oy);
			int dstX = std::max(0, ox);
			int dstY = std::max(0, oy);
			int copyW = std::min(img.cols - srcX, cellW - dstX);
			int copyH = std::min(img.rows - srcY, cellH - dstY);
			if (copyW > 0 && copyH > 0)
			{
				img(cv::Rect(srcX, srcY, copyW, copyH))
					.copyTo(cell(cv::Rect(dstX, dstY, copyW, copyH)));
			}
		}
		else
		{
			// 等比缩放适配
			double scaleW = static_cast<double>(cellW) / img.cols;
			double scaleH = static_cast<double>(cellH) / img.rows;
			double scale = std::min(scaleW, scaleH);
			int newW = safeRound((img.cols * scale));
			int newH = safeRound((img.rows * scale));
			cv::Mat resized;
			cv::resize(img, resized, cv::Size(newW, newH));
			cell = cv::Mat(cellH, cellW, CV_8UC3, bgScalar);
			int ox = (cellW - newW) / 2;
			int oy = (cellH - newH) / 2;
			resized.copyTo(cell(cv::Rect(ox, oy, newW, newH)));
		}

		cell.copyTo(canvas(cv::Rect(x0, y0, cellW, cellH)));
	}

	setOutput("output_image", NodeData::createImage(canvas));
}
