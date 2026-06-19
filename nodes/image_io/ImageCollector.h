// nodes/image_io/ImageCollector.h
#pragma once
#include "NodeBase.h"
class ImageCollector : public NodeBase {
public:
	ImageCollector();
	void defineNode() override;
	void process() override;
	void onInputConnected(const QString& portName, DataType actualType) override;
	void onInputDisconnected(const QString& portName) override;
private:
	int m_nextPort;
};
