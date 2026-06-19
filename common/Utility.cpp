// common/Utility.cpp
#include "Utility.h"
#include <opencv2/imgproc.hpp>

namespace Utility
{

	QImage matToQImage(const cv::Mat& mat)
	{
		if (mat.empty())
		{
			return QImage();
		}

		switch (mat.type())
		{
		case CV_8UC1:
		{
			QImage img(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
			return img.copy();
		}
		case CV_8UC3:
		{
			cv::Mat rgb;
			cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
			QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
			return img.copy();
		}
		case CV_8UC4:
		{
			cv::Mat rgba;
			cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
			QImage img(rgba.data, rgba.cols, rgba.rows, static_cast<int>(rgba.step), QImage::Format_RGBA8888);
			return img.copy();
		}
		default:
		{
			// 尝试转为8UC3再处理
			cv::Mat converted;
			mat.convertTo(converted, CV_8UC3);
			return matToQImage(converted);
		}
		}
	}

	cv::Mat qImageToMat(const QImage& image)
	{
		if (image.isNull())
		{
			return cv::Mat();
		}

		QImage converted;
		switch (image.format())
		{
		case QImage::Format_RGB888:
			converted = image;
			break;
		case QImage::Format_RGBA8888:
		case QImage::Format_RGBA8888_Premultiplied:
			converted = image.convertToFormat(QImage::Format_RGBA8888);
			break;
		case QImage::Format_Grayscale8:
		{
			cv::Mat gray(image.height(), image.width(), CV_8UC1,
						 const_cast<uchar*>(image.bits()),
						 static_cast<size_t>(image.bytesPerLine()));
			return gray.clone();
		}
		default:
			// 其他格式统一转为RGBA
			converted = image.convertToFormat(QImage::Format_RGBA8888);
			break;
		}

		if (converted.format() == QImage::Format_RGB888)
		{
			cv::Mat rgb(converted.height(), converted.width(), CV_8UC3,
						const_cast<uchar*>(converted.bits()),
						static_cast<size_t>(converted.bytesPerLine()));
			cv::Mat bgr;
			cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
			return bgr.clone();
		}
		else
		{
			cv::Mat rgba(converted.height(), converted.width(), CV_8UC4,
						 const_cast<uchar*>(converted.bits()),
						 static_cast<size_t>(converted.bytesPerLine()));
			cv::Mat bgra;
			cv::cvtColor(rgba, bgra, cv::COLOR_RGBA2BGRA);
			return bgra.clone();
		}
	}

	cv::Scalar colorToScalar(const QColor& color, bool withAlpha)
	{
		if (withAlpha)
		{
			return cv::Scalar(color.blue(), color.green(), color.red(), color.alpha());
		}
		return cv::Scalar(color.blue(), color.green(), color.red());
	}

	QColor arrayToColor(const QVariantList& arr)
	{
		if (arr.size() < 3)
		{
			return QColor(Qt::white);
		}
		int r = arr[0].toInt();
		int g = arr[1].toInt();
		int b = arr[2].toInt();
		int a = (arr.size() >= 4) ? arr[3].toInt() : 255;
		return QColor(r, g, b, a);
	}

	QVariantList colorToArray(const QColor& color)
	{
		return QVariantList{ color.red(), color.green(), color.blue(), color.alpha() };
	}

	int ensureOdd(int value)
	{
		if (value < 1)
		{
			return 1;
		}
		if (value % 2 == 0)
		{
			return value + 1;
		}
		return value;
	}

	QString formatBytes(qint64 bytes)
	{
		if (bytes < 1024)
		{
			return QString("%1 B").arg(bytes);
		}
		if (bytes < 1024 * 1024)
		{
			return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
		}
		if (bytes < 1024LL * 1024 * 1024)
		{
			return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
		}
		return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
	}

} // namespace Utility
