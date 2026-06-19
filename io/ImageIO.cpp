// io/ImageIO.cpp
#include "ImageIO.h"
#include "Utility.h"
#include "xlsxdocument.h"
#include <opencv2/imgcodecs.hpp>
#include <QFileInfo>
#include <QDir>

cv::Mat ImageIO::readImage(const QString& filePath)
{
	return cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
}

bool ImageIO::writeImage(const QString& filePath, const cv::Mat& mat)
{
	QFileInfo fi(filePath);
	QDir dir = fi.absoluteDir();
	if (!dir.exists()) dir.mkpath(".");
	return cv::imwrite(filePath.toStdString(), mat);
}

QImage ImageIO::matToQImage(const cv::Mat& mat)
{
	return Utility::matToQImage(mat);
}

cv::Mat ImageIO::qImageToMat(const QImage& image)
{
	return Utility::qImageToMat(image);
}
