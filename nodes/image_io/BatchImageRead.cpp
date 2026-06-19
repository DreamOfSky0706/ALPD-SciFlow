// nodes/image_io/BatchImageRead.cpp
#include "BatchImageRead.h"
#include <opencv2/imgcodecs.hpp>
#include <QDir>
#include <QFileInfoList>
#include <algorithm>

void BatchImageRead::defineNode()
{
	addOutputPort("output_images", DataType::ImageList);

	addParam("dir_path", "文件夹路径", ParamType::DirPath, QString());
	addParam("pattern", "文件名通配符", ParamType::String, QString("*.png"),
			 { {"placeholder", "如 *.png;*.jpg"} });
	addParam("sort_mode", "排序方式", ParamType::Combo, QString("按文件名升序"),
			 { {"options", QStringList{"按文件名升序", "按文件名降序",
									  "按修改时间升序", "按修改时间降序"}} });
}

void BatchImageRead::process()
{
	QString dirPath = param("dir_path").toString();
	if (dirPath.isEmpty())
	{
		reportError("未指定文件夹路径");
		return;
	}

	QDir dir(dirPath);
	if (!dir.exists())
	{
		reportError(QString("文件夹不存在：%1").arg(dirPath));
		return;
	}

	// 解析通配符，支持分号分隔多个模式
	QString pattern = param("pattern").toString();
	QStringList filters = pattern.split(";", Qt::SkipEmptyParts);
	for (auto& f : filters)
	{
		f = f.trimmed();
	}

	dir.setNameFilters(filters);
	dir.setFilter(QDir::Files | QDir::Readable);

	// 排序
	QString sortMode = param("sort_mode").toString();
	QDir::SortFlags sortFlags = QDir::Name;
	if (sortMode == "按文件名降序")
	{
		sortFlags = QDir::Name | QDir::Reversed;
	}
	else if (sortMode == "按修改时间升序")
	{
		sortFlags = QDir::Time | QDir::Reversed;
	}
	else if (sortMode == "按修改时间降序")
	{
		sortFlags = QDir::Time;
	}
	dir.setSorting(sortFlags);

	QFileInfoList fileList = dir.entryInfoList();
	if (fileList.isEmpty())
	{
		reportWarning("未找到匹配的图片文件");
		setOutput("output_images", NodeData::createImageList({}));
		return;
	}

	std::vector<cv::Mat> images;
	int failCount = 0;

	for (const QFileInfo& fi : fileList)
	{
		cv::Mat img = cv::imread(fi.absoluteFilePath().toStdString(), cv::IMREAD_UNCHANGED);
		if (img.empty())
		{
			reportWarning(QString("无法读取文件：%1，已跳过").arg(fi.fileName()));
			failCount++;
			continue;
		}
		if (img.depth() == CV_16U)
		{
			img.convertTo(img, CV_8U, 255.0 / 65535.0);
		}
		images.push_back(img);
	}

	if (failCount > 0)
	{
		reportWarning(QString("共%1个文件读取失败").arg(failCount));
	}

	reportWarning(QString("成功读取%1张图片").arg(images.size()));
	setOutput("output_images", NodeData::createImageList(images));
}
