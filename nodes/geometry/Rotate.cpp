// nodes/geometry/Rotate.cpp
#include "Rotate.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

void Rotate::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("angle", "旋转角度", ParamType::DoubleSlider, 0.0,
			 { {"min", -360.0}, {"max", 360.0} });
	addParam("expand_canvas", "扩展画布", ParamType::Bool, true);
	addParam("fill_color", "填充颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
}

void Rotate::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	double angle = param("angle").toDouble();
	bool expand = param("expand_canvas").toBool();
	QColor fillColor = Utility::arrayToColor(param("fill_color").toList());

	cv::Point2f center(src.cols / 2.0f, src.rows / 2.0f);
	cv::Mat rotMat = cv::getRotationMatrix2D(center, -angle, 1.0);

	int dstW = src.cols;
	int dstH = src.rows;

	if (expand)
	{
		// 计算旋转后的包围矩形尺寸
		double absAngle = std::abs(angle * CV_PI / 180.0);
		double sinA = std::abs(std::sin(absAngle));
		double cosA = std::abs(std::cos(absAngle));
		dstW = static_cast<int>(std::ceil(src.cols * cosA + src.rows * sinA));
		dstH = static_cast<int>(std::ceil(src.cols * sinA + src.rows * cosA));

		// 调整变换矩阵的平移分量
		rotMat.at<double>(0, 2) += (dstW - src.cols) / 2.0;
		rotMat.at<double>(1, 2) += (dstH - src.rows) / 2.0;
	}

	cv::Scalar border = Utility::colorToScalar(fillColor, src.channels() == 4);
	cv::Mat dst;
	cv::warpAffine(src, dst, rotMat, cv::Size(dstW, dstH),
				   cv::INTER_LINEAR, cv::BORDER_CONSTANT, border);

	setOutput("output_image", NodeData::createImage(dst));
}
