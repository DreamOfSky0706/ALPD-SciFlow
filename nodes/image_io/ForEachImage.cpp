// nodes/image_io/ForEachImage.cpp
#include "ForEachImage.h"
#include "Logger.h"
#include <opencv2/imgproc.hpp>

void ForEachImage::defineNode()
{
	addInputPort("input_images", DataType::ImageList);
	addOutputPort("output_images", DataType::ImageList);

	addParam("operation", "操作类型", ParamType::Combo, QString("缩放"),
			 {{"options",QStringList{"缩放","旋转","翻转","转灰度","高斯模糊","亮度调整","锐化","反色"}}});
	addParam("param1", "参数1", ParamType::Double, 0.5,
			 {{"label","缩放比例/角度/核大小/亮度值"}});
	addParam("param2", "参数2", ParamType::Double, 0.0,
			 {{"label","辅助参数（多数操作不需要）"}});
}

void ForEachImage::process()
{
	auto data = getInput("input_images");
	if (!data || data->isNull()) { reportError("输入图像列表为空"); return; }

	std::vector<cv::Mat> srcList = data->toImageList();
	if (srcList.empty()) { reportError("图像列表为空"); return; }

	QString op = param("operation").toString();
	double p1 = param("param1").toDouble();
	double p2 = param("param2").toDouble();
	std::vector<cv::Mat> results;

	for (size_t i = 0; i < srcList.size(); ++i) {
		cv::Mat src = srcList[i], dst;
		if (src.empty()) continue;

		if (op == "缩放") {
			cv::resize(src, dst, cv::Size(), p1, p1, cv::INTER_LINEAR);
		} else if (op == "旋转") {
			cv::Point2f c(src.cols/2.f, src.rows/2.f);
			cv::Mat M = cv::getRotationMatrix2D(c, p1, 1.0);
			cv::warpAffine(src, dst, M, src.size());
		} else if (op == "翻转") {
			int code = (p1 < 0.5) ? 1 : ((p1 < 1.5) ? 0 : -1);
			cv::flip(src, dst, code);
		} else if (op == "转灰度") {
			if (src.channels() > 1) cv::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
			else dst = src.clone();
		} else if (op == "高斯模糊") {
			int ks = qBound(1, (int)p1, 99);
			if (ks % 2 == 0) ks++;
			cv::GaussianBlur(src, dst, cv::Size(ks, ks), p2 > 0 ? p2 : 1.0);
		} else if (op == "亮度调整") {
			src.convertTo(dst, -1, 1.0, p1);
		} else if (op == "锐化") {
			cv::Mat blurred;
			cv::GaussianBlur(src, blurred, cv::Size(3,3), 1.0);
			cv::addWeighted(src, 1.0 + p1, blurred, -p1, 0, dst);
		} else if (op == "反色") {
			cv::bitwise_not(src, dst);
		} else {
			dst = src.clone();
		}
		results.push_back(dst);
	}

	Logger::instance().success(QString("批次处理完成: %1张图片, 操作=%2").arg(results.size()).arg(op));
	setOutput("output_images", NodeData::createImageList(results));
}
