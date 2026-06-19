// nodes/pattern/CustomAssetImport.cpp
#include "CustomAssetImport.h"
#include <opencv2/imgcodecs.hpp>
#include <QFileInfo>

void CustomAssetImport::defineNode()
{
	addOutputPort("output_image", DataType::Image);
	addParam("file_path", "素材文件路径", ParamType::FilePath, QString(),
			 { {"filter", "图片文件 (*.png *.jpg *.bmp *.webp)"} });
}

void CustomAssetImport::process()
{
	QString path = param("file_path").toString();
	if (path.isEmpty())
	{
		reportError("未指定素材文件路径"); return;
	}
	if (!QFileInfo::exists(path))
	{
		reportError(QString("文件不存在：%1").arg(path)); return;
	}
	cv::Mat img = cv::imread(path.toStdString(), cv::IMREAD_UNCHANGED);
	if (img.empty())
	{
		reportError(QString("无法读取文件：%1").arg(path)); return;
	}
	if (img.depth() == CV_16U) img.convertTo(img, CV_8U, 255.0 / 65535.0);
	setOutput("output_image", NodeData::createImage(img));
}
