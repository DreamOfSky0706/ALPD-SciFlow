// nodes/image_io/CheckerboardGenerate.cpp
#include "CheckerboardGenerate.h"
#include "Utility.h"

void CheckerboardGenerate::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("width", "宽度", ParamType::Int, 800, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 600, { {"min", 1}, {"max", 20000} });
	addParam("cell_size", "格子大小", ParamType::IntSlider, 50, { {"min", 1}, {"max", 500} });
	addParam("color_a", "颜色A", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("color_b", "颜色B", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
}

void CheckerboardGenerate::process()
{
	int w = param("width").toInt();
	int h = param("height").toInt();
	int cellSize = param("cell_size").toInt();

	if (w < 1 || w > 20000 || h < 1 || h > 20000)
	{
		reportError(QString("图像尺寸超出范围：%1x%2").arg(w).arg(h));
		return;
	}
	if (cellSize < 1)
	{
		cellSize = 1;
	}

	QColor colA = Utility::arrayToColor(param("color_a").toList());
	QColor colB = Utility::arrayToColor(param("color_b").toList());
	cv::Scalar scA = Utility::colorToScalar(colA, false);
	cv::Scalar scB = Utility::colorToScalar(colB, false);

	cv::Mat img(h, w, CV_8UC3);

	for (int y = 0; y < h; ++y)
	{
		uchar* row = img.ptr<uchar>(y);
		int gy = y / cellSize;
		for (int x = 0; x < w; ++x)
		{
			int gx = x / cellSize;
			bool isA = ((gx + gy) % 2 == 0);
			const cv::Scalar& c = isA ? scA : scB;
			int idx = x * 3;
			row[idx] = static_cast<uchar>(c[0]);
			row[idx + 1] = static_cast<uchar>(c[1]);
			row[idx + 2] = static_cast<uchar>(c[2]);
		}
	}

	setOutput("output_image", NodeData::createImage(img));
}
