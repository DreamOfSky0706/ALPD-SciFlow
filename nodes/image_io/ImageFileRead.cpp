// nodes/image_io/ImageFileRead.cpp
#include "ImageFileRead.h"
#include <opencv2/imgcodecs.hpp>
#include <QFileInfo>

void ImageFileRead::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("file_path", "文件路径", ParamType::FilePath, QString(),
			 { {"filter", "图片文件 (*.png *.jpg *.jpeg *.tiff *.tif *.bmp *.webp)"} });
}

void ImageFileRead::process()
{
	QString filePath = param("file_path").toString();
	if (filePath.isEmpty())
	{
		reportError("未指定输入文件路径");
		return;
	}

	QFileInfo fi(filePath);
	if (!fi.exists())
	{
		reportError(QString("文件不存在：%1").arg(filePath));
		return;
	}

	cv::Mat img = cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
	if (img.empty())
	{
		reportError(QString("无法解析图像文件：%1").arg(filePath));
		return;
	}

	// 16位深度转为8位
	if (img.depth() == CV_16U)
	{
		img.convertTo(img, CV_8U, 255.0 / 65535.0);
		reportWarning("原图为16位深度，已自动转换为8位");
	}

	setOutput("output_image", NodeData::createImage(img));
}
