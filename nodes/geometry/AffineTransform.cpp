// nodes/geometry/AffineTransform.cpp
#include "AffineTransform.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>

void AffineTransform::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("src_x0", "源点1 X", ParamType::Double, 0.0, { {"min", -99999}, {"max", 99999} });
	addParam("src_y0", "源点1 Y", ParamType::Double, 0.0, { {"min", -99999}, {"max", 99999} });
	addParam("src_x1", "源点2 X", ParamType::Double, 100.0, { {"min", -99999}, {"max", 99999} });
	addParam("src_y1", "源点2 Y", ParamType::Double, 0.0, { {"min", -99999}, {"max", 99999} });
	addParam("src_x2", "源点3 X", ParamType::Double, 0.0, { {"min", -99999}, {"max", 99999} });
	addParam("src_y2", "源点3 Y", ParamType::Double, 100.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_x0", "目标点1 X", ParamType::Double, 10.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_y0", "目标点1 Y", ParamType::Double, 10.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_x1", "目标点2 X", ParamType::Double, 110.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_y1", "目标点2 Y", ParamType::Double, 20.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_x2", "目标点3 X", ParamType::Double, 20.0, { {"min", -99999}, {"max", 99999} });
	addParam("dst_y2", "目标点3 Y", ParamType::Double, 110.0, { {"min", -99999}, {"max", 99999} });
	addParam("output_width", "输出宽度", ParamType::Int, 0,
			 { {"min", 0}, {"max", 20000} });
	addParam("output_height", "输出高度", ParamType::Int, 0,
			 { {"min", 0}, {"max", 20000} });
	addParam("fill_color", "填充颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
}

void AffineTransform::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();

	cv::Point2f srcPts[3];
	srcPts[0] = cv::Point2f(param("src_x0").toFloat(), param("src_y0").toFloat());
	srcPts[1] = cv::Point2f(param("src_x1").toFloat(), param("src_y1").toFloat());
	srcPts[2] = cv::Point2f(param("src_x2").toFloat(), param("src_y2").toFloat());

	cv::Point2f dstPts[3];
	dstPts[0] = cv::Point2f(param("dst_x0").toFloat(), param("dst_y0").toFloat());
	dstPts[1] = cv::Point2f(param("dst_x1").toFloat(), param("dst_y1").toFloat());
	dstPts[2] = cv::Point2f(param("dst_x2").toFloat(), param("dst_y2").toFloat());

	cv::Mat warpMat = cv::getAffineTransform(srcPts, dstPts);

	int outW = param("output_width").toInt();
	int outH = param("output_height").toInt();
	if (outW <= 0) outW = src.cols;
	if (outH <= 0) outH = src.rows;

	QColor fill = Utility::arrayToColor(param("fill_color").toList());
	cv::Scalar border = Utility::colorToScalar(fill, src.channels() == 4);

	cv::Mat dst;
	cv::warpAffine(src, dst, warpMat, cv::Size(outW, outH),
				   cv::INTER_LINEAR, cv::BORDER_CONSTANT, border);

	setOutput("output_image", NodeData::createImage(dst));
}
