// nodes/layout/ImagePreview.cpp
#include "ImagePreview.h"
#include "Logger.h"

void ImagePreview::defineNode()
{
	addInputPort("input_image", DataType::Image);

	addParam("window_title", "预览标题", ParamType::String, QString("预览"));
}

void ImagePreview::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportWarning("预览输入为空");
		return;
	}

	// GUI模式下由预览面板读取此节点的输入数据进行显示
	// CLI模式下此节点不做任何操作
	Logger::instance().info(
		QString("预览节点[%1]：图像已就绪").arg(param("window_title").toString()));
}
