// nodes/image_io/ImageCollector.cpp
#include "ImageCollector.h"
#include "Logger.h"

ImageCollector::ImageCollector() : m_nextPort(0) {}

void ImageCollector::defineNode()
{
	if (m_nextPort == 0) {
		addInputPort("input_0", DataType::Image, false);
		addInputPort("input_1", DataType::Image, false);
		m_nextPort = 2;
	} else {
		// rebuildPorts场景：已有m_nextPort确定的端口数
		for (int i = 0; i < m_nextPort; ++i)
			addInputPort(QString("input_%1").arg(i), DataType::Image, false);
	}
	addOutputPort("output_images", DataType::ImageList);
}

void ImageCollector::process()
{
	std::vector<cv::Mat> images;
	for (int i = 0; i < m_nextPort; ++i) {
		auto d = getInput(QString("input_%1").arg(i));
		if (d && !d->isNull())
			images.push_back(d->toImage());
	}
	Logger::instance().info(QString("收集%1张图片为一批").arg(images.size()));
	setOutput("output_images", NodeData::createImageList(images));
}

void ImageCollector::onInputConnected(const QString& portName, DataType)
{
	int idx = portName.mid(6).toInt();
	if (idx == m_nextPort - 1) {
		// 最后一个端口被连接→追加一个新端口（直接add，不rebuild破坏已有端口）
		m_nextPort++;
		addInputPort(QString("input_%1").arg(m_nextPort - 1), DataType::Image, false);
	}
}

void ImageCollector::onInputDisconnected(const QString& portName)
{
	Q_UNUSED(portName)
	// 简化：不自动收缩，避免影响已有连线
}
