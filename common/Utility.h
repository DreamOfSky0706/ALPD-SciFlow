// common/Utility.h
#pragma once

#include <QImage>
#include <QColor>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QFont>
#include <QPainterPath>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
inline int safeRound(double v)
{
	return static_cast<int>(std::round(v));
}

namespace Utility
{

	// cv::Mat(BGR/BGRA/Gray) 转 QImage
	QImage matToQImage(const cv::Mat& mat);

	// QImage 转 cv::Mat(BGR/BGRA)
	cv::Mat qImageToMat(const QImage& image);

	// QColor 转 cv::Scalar(BGR或BGRA)
	cv::Scalar colorToScalar(const QColor& color, bool withAlpha = false);

	// RGBA整数数组转QColor
	QColor arrayToColor(const QVariantList& arr);

	// QColor转RGBA整数数组
	QVariantList colorToArray(const QColor& color);

	// 确保核大小为奇数
	int ensureOdd(int value);

	// 将字节大小格式化为可读字符串
	QString formatBytes(qint64 bytes);

} // namespace Utility
