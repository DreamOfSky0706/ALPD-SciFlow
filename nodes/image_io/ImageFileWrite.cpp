// nodes/image_io/ImageFileWrite.cpp
#include "ImageFileWrite.h"
#include "Utility.h"
#include "Logger.h"
#include <opencv2/imgcodecs.hpp>
#include <QFileInfo>
#include <QDir>
#include <QImage>

void ImageFileWrite::defineNode()
{
	addInputPort("input_image", DataType::Image);

	addParam("file_path", "输出路径", ParamType::SaveFilePath,
			 QDir::currentPath() + "/output.png",
			 { {"filter", "PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tiff *.tif);;BMP (*.bmp)"} });
	addParam("format", "格式", ParamType::Combo, QString("PNG"),
			 { {"options", QStringList{"PNG", "JPEG", "TIFF", "BMP"}} });
	addParam("jpeg_quality", "JPEG质量", ParamType::IntSlider, 95,
			 { {"min", 1}, {"max", 100}, {"visible_when", "format==JPEG"} });
	addParam("png_compression", "PNG压缩级别", ParamType::IntSlider, 3,
			 { {"min", 0}, {"max", 9}, {"visible_when", "format==PNG"} });
}

void ImageFileWrite::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	QString filePath = param("file_path").toString();
	if (filePath.isEmpty())
	{
		reportError("未指定输出文件路径");
		return;
	}

	// 确保输出目录存在
	QFileInfo fi(filePath);
	QDir dir = fi.absoluteDir();
	if (!dir.exists())
	{
		if (!dir.mkpath("."))
		{
			reportError(QString("无法创建输出目录：%1").arg(dir.absolutePath()));
			return;
		}
	}

	cv::Mat img = inputData->toImage();
	QString format = param("format").toString();

	// JPEG不支持Alpha通道
	if (format == "JPEG" && img.channels() == 4)
	{
		cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
		reportWarning("JPEG格式不支持Alpha通道，已自动移除");
	}

	// 使用QImage保存以写入DPI元信息
	QImage qimg = Utility::matToQImage(img);

	// 这里暂用默认DPI，后续可通过上游节点传递DPI信息
	qimg.setDotsPerMeterX(safeRound((300 * 39.3701)));
	qimg.setDotsPerMeterY(safeRound((300 * 39.3701)));

	// 确定文件扩展名
	QString suffix = fi.suffix().toLower();
	if (suffix.isEmpty())
	{
		if (format == "PNG") filePath += ".png";
		else if (format == "JPEG") filePath += ".jpg";
		else if (format == "TIFF") filePath += ".tiff";
		else if (format == "BMP") filePath += ".bmp";
	}

	bool success = false;
	if (format == "JPEG")
	{
		int quality = param("jpeg_quality").toInt();
		success = qimg.save(filePath, "JPEG", quality);
	}
	else if (format == "PNG")
	{
		// QImage::save的质量参数对PNG是压缩级别映射
		// Qt文档：对PNG，quality从0(无压缩)到100(最大压缩)
		int compression = param("png_compression").toInt();
		int qtQuality = safeRound((compression / 9.0 * 100.0));
		success = qimg.save(filePath, "PNG", qtQuality);
	}
	else if (format == "TIFF")
	{
		success = qimg.save(filePath, "TIFF");
	}
	else
	{
		success = qimg.save(filePath, "BMP");
	}

	if (!success)
	{
		reportError(QString("写入文件失败：%1，请检查路径是否可写").arg(filePath));
		return;
	}

	Logger::instance().success(QString("图片已导出: %1").arg(filePath));
}
