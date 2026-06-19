// nodes/image_io/BatchImageWrite.cpp
#include "BatchImageWrite.h"
#include "Utility.h"
#include "Logger.h"
#include <opencv2/imgcodecs.hpp>
#include <QDir>
#include <QImage>
#include <cmath>

void BatchImageWrite::defineNode()
{
	addInputPort("input_images", DataType::ImageList);

	addParam("dir_path", "输出目录", ParamType::DirPath, QString());
	addParam("name_template", "文件名模板", ParamType::String, QString("output_{index}"),
			 { {"placeholder", "使用{index}作为序号占位符"} });
	addParam("format", "格式", ParamType::Combo, QString("PNG"),
			 { {"options", QStringList{"PNG", "JPEG", "TIFF", "BMP"}} });
	addParam("jpeg_quality", "JPEG质量", ParamType::IntSlider, 95,
			 { {"min", 1}, {"max", 100}, {"visible_when", "format==JPEG"} });
}

void BatchImageWrite::process()
{
	auto inputData = getInput("input_images");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像列表为空");
		return;
	}

	QString dirPath = param("dir_path").toString();
	if (dirPath.isEmpty())
	{
		reportError("未指定输出目录");
		return;
	}

	QDir dir(dirPath);
	if (!dir.exists())
	{
		if (!dir.mkpath("."))
		{
			reportError(QString("无法创建输出目录：%1").arg(dirPath));
			return;
		}
	}

	std::vector<cv::Mat> images = inputData->toImageList();
	if (images.empty())
	{
		reportWarning("图像列表为空，无内容可导出");
		return;
	}

	QString nameTemplate = param("name_template").toString();
	QString format = param("format").toString();
	int jpegQuality = param("jpeg_quality").toInt();

	// 计算序号补零位数
	int digits = static_cast<int>(std::ceil(std::log10(static_cast<double>(images.size()) + 1)));
	if (digits < 1) digits = 1;

	QString suffix;
	if (format == "PNG") suffix = ".png";
	else if (format == "JPEG") suffix = ".jpg";
	else if (format == "TIFF") suffix = ".tiff";
	else suffix = ".bmp";

	int failCount = 0;

	for (size_t i = 0; i < images.size(); ++i)
	{
		QString indexStr = QString("%1").arg(i, digits, 10, QChar('0'));
		QString fileName = nameTemplate;
		fileName.replace("{index}", indexStr);
		fileName += suffix;

		QString filePath = dir.absoluteFilePath(fileName);

		cv::Mat img = images[i];
		if (format == "JPEG" && img.channels() == 4)
		{
			cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
		}

		QImage qimg = Utility::matToQImage(img);
		bool ok = false;

		if (format == "JPEG")
		{
			ok = qimg.save(filePath, "JPEG", jpegQuality);
		}
		else
		{
			ok = qimg.save(filePath, format.toUtf8().constData());
		}

		if (!ok)
		{
			reportWarning(QString("写入失败：%1").arg(filePath));
			failCount++;
		}
	}

	if (failCount > 0)
	{
		reportWarning(QString("共%1张图片写入失败").arg(failCount));
	}
	else
	{
		Logger::instance().success(QString("批量导出%1张图片到: %2").arg(images.size()).arg(dirPath));
	}
}
