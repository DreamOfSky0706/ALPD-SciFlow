// nodes/composite/ImageWatermark.cpp
#include "ImageWatermark.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void ImageWatermark::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addInputPort("input_watermark", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("position", "位置", ParamType::Combo, QString("右下"),
			 { {"options", QStringList{"左上", "上中", "右上", "左中", "正中", "右中",
									  "左下", "下中", "右下", "自定义坐标"}} });
	addParam("custom_x", "自定义X", ParamType::Int, 0,
			 { {"min", 0}, {"max", 20000}, {"visible_when", "position==自定义坐标"} });
	addParam("custom_y", "自定义Y", ParamType::Int, 0,
			 { {"min", 0}, {"max", 20000}, {"visible_when", "position==自定义坐标"} });
	addParam("scale", "缩放比例", ParamType::DoubleSlider, 0.2,
			 { {"min", 0.01}, {"max", 5.0} });
	addParam("opacity", "不透明度", ParamType::DoubleSlider, 0.5,
			 { {"min", 0.0}, {"max", 1.0} });
	addParam("tiling", "平铺", ParamType::Bool, false);
}

void ImageWatermark::process()
{
	auto imgData = getInput("input_image");
	auto wmData = getInput("input_watermark");

	if (!imgData || imgData->isNull())
	{
		reportError("输入图像为空");
		return;
	}
	if (!wmData || wmData->isNull())
	{
		reportError("水印图像为空");
		return;
	}

	cv::Mat img = imgData->toImage();
	cv::Mat wm = wmData->toImage();
	double scale = param("scale").toDouble();
	double opacity = param("opacity").toDouble();
	bool tiling = param("tiling").toBool();

	// 确保主图至少三通道
	if (img.channels() == 1)
	{
		cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
	}

	cv::Mat dst = img.clone();

	// 缩放水印
	int wmW = safeRound((img.cols * scale));
	int wmH = safeRound((wm.rows * (static_cast<double>(wmW) / wm.cols)));
	cv::Mat wmResized;
	cv::resize(wm, wmResized, cv::Size(wmW, wmH));

	// 分离水印的Alpha（如有）
	cv::Mat wmBGR, wmAlpha;
	if (wmResized.channels() == 4)
	{
		std::vector<cv::Mat> ch;
		cv::split(wmResized, ch);
		wmAlpha = ch[3];
		cv::merge(std::vector<cv::Mat>{ch[0], ch[1], ch[2]}, wmBGR);
	}
	else
	{
		if (wmResized.channels() == 1)
		{
			cv::cvtColor(wmResized, wmBGR, cv::COLOR_GRAY2BGR);
		}
		else
		{
			wmBGR = wmResized;
		}
		wmAlpha = cv::Mat(wmH, wmW, CV_8UC1, cv::Scalar(255));
	}

	// 内部函数：在指定坐标叠加一次水印
	auto overlayOnce = [&](int ox, int oy)
		{
			// 计算有效拷贝区域
			int srcX = std::max(0, -ox);
			int srcY = std::max(0, -oy);
			int dstX = std::max(0, ox);
			int dstY = std::max(0, oy);
			int copyW = std::min(wmW - srcX, dst.cols - dstX);
			int copyH = std::min(wmH - srcY, dst.rows - dstY);
			if (copyW <= 0 || copyH <= 0) return;

			for (int y = 0; y < copyH; ++y)
			{
				const uchar* wmRow = wmBGR.ptr<uchar>(srcY + y);
				const uchar* alphaRow = wmAlpha.ptr<uchar>(srcY + y);
				uchar* dstRow = dst.ptr<uchar>(dstY + y);
				int ch = dst.channels();

				for (int x = 0; x < copyW; ++x)
				{
					double a = (alphaRow[srcX + x] / 255.0) * opacity;
					int di = (dstX + x) * ch;
					int si = (srcX + x) * 3;

					for (int c = 0; c < std::min(ch, 3); ++c)
					{
						double orig = dstRow[di + c];
						double watermark = wmRow[si + c];
						dstRow[di + c] = static_cast<uchar>(
							std::clamp(orig * (1.0 - a) + watermark * a, 0.0, 255.0));
					}
				}
			}
		};

	if (tiling)
	{
		for (int ty = 0; ty < dst.rows; ty += wmH)
		{
			for (int tx = 0; tx < dst.cols; tx += wmW)
			{
				overlayOnce(tx, ty);
			}
		}
	}
	else
	{
		// 计算位置
		QString pos = param("position").toString();
		int ox = 0;
		int oy = 0;

		if (pos == "自定义坐标")
		{
			ox = param("custom_x").toInt();
			oy = param("custom_y").toInt();
		}
		else
		{
			int margin = 10;
			if (pos.contains("左")) ox = margin;
			else if (pos.contains("右")) ox = dst.cols - wmW - margin;
			else ox = (dst.cols - wmW) / 2;

			if (pos.contains("上")) oy = margin;
			else if (pos.contains("下")) oy = dst.rows - wmH - margin;
			else oy = (dst.rows - wmH) / 2;
		}

		overlayOnce(ox, oy);
	}

	setOutput("output_image", NodeData::createImage(dst));
}
