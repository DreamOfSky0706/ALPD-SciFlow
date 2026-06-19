// nodes/geometry/Flip.cpp
#include "Flip.h"
#include <opencv2/core.hpp>

void Flip::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("direction", "翻转方向", ParamType::Combo, QString("水平翻转"),
			 { {"options", QStringList{"水平翻转", "垂直翻转", "水平+垂直翻转"}} });
}

void Flip::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空");
		return;
	}

	cv::Mat src = inputData->toImage();
	QString dir = param("direction").toString();

	int flipCode = 1;
	if (dir == "垂直翻转") flipCode = 0;
	else if (dir == "水平+垂直翻转") flipCode = -1;

	cv::Mat dst;
	cv::flip(src, dst, flipCode);
	setOutput("output_image", NodeData::createImage(dst));
}
