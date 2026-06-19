// nodes/layout/ImageDiff.cpp
#include "ImageDiff.h"
#include <opencv2/imgproc.hpp>

void ImageDiff::defineNode()
{
	addInputPort("input_a", DataType::Image);
	addInputPort("input_b", DataType::Image);
	addOutputPort("output_image", DataType::Image);
	addOutputPort("diff_ratio", DataType::Numeric);

	addParam("mode", "模式", ParamType::Combo, QString("差值图"),
			 {{"options",QStringList{"差值图","并排对比","闪烁对比","差异蒙版"}}});
	addParam("threshold", "差异阈值", ParamType::IntSlider, 30,
			 {{"min",1},{"max",128}});
}

void ImageDiff::process()
{
	auto da = getInput("input_a");
	auto db = getInput("input_b");
	if (!da||da->isNull()||!db||db->isNull()) { reportError("输入图像为空"); return; }

	cv::Mat a = da->toImage(), b = db->toImage();
	// 统一尺寸和通道数
	if (a.size() != b.size()) cv::resize(b, b, a.size());
	if (a.channels() != b.channels()) {
		if (a.channels() == 1) cv::cvtColor(a, a, cv::COLOR_GRAY2BGR);
		if (b.channels() == 1) cv::cvtColor(b, b, cv::COLOR_GRAY2BGR);
		if (a.channels() == 4) cv::cvtColor(a, a, cv::COLOR_BGRA2BGR);
		if (b.channels() == 4) cv::cvtColor(b, b, cv::COLOR_BGRA2BGR);
	}

	QString mode = param("mode").toString();
	int thresh = param("threshold").toInt();
	cv::Mat result;

	if (mode == "差值图") {
		cv::absdiff(a, b, result);
		if (result.channels() == 1) cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
		// 放大差异
		result.convertTo(result, -1, 3.0);
	} else if (mode == "并排对比") {
		int w = a.cols, h = a.rows;
		result = cv::Mat(h, w*2+10, a.type(), cv::Scalar(128,128,128));
		a.copyTo(result(cv::Rect(0,0,w,h)));
		b.copyTo(result(cv::Rect(w+10,0,w,h)));
	} else if (mode == "闪烁对比") {
		// 输出交替图（上下拼接表示闪烁概念）
		cv::vconcat(a, b, result);
		// 在中间画分隔线
		cv::line(result, cv::Point(0,a.rows), cv::Point(result.cols,a.rows), cv::Scalar(0,255,0), 2);
	} else {
		// 差异蒙版
		cv::Mat diff;
		cv::absdiff(a, b, diff);
		if (diff.channels() > 1) cv::cvtColor(diff, diff, cv::COLOR_BGR2GRAY);
		cv::threshold(diff, result, thresh, 255, cv::THRESH_BINARY);
	}

	// 计算差异比例
	cv::Mat diffGray;
	cv::absdiff(a, b, diffGray);
	if (diffGray.channels() > 1) cv::cvtColor(diffGray, diffGray, cv::COLOR_BGR2GRAY);
	double ratio = (double)cv::countNonZero(diffGray > thresh) / diffGray.total();

	setOutput("output_image", NodeData::createImage(result));
	setOutput("diff_ratio", NodeData::createNumeric(ratio));
}
