// io/ImageIO.h
#pragma once

#include <QString>
#include <opencv2/core.hpp>
#include <QImage>

class ImageIO
{
public:
	static cv::Mat readImage(const QString& filePath);
	static bool writeImage(const QString& filePath, const cv::Mat& mat);
	static QImage matToQImage(const cv::Mat& mat);
	static cv::Mat qImageToMat(const QImage& image);
};
