// nodes/chart/Histogram.h
#pragma once

#include "NodeBase.h"

// 直方图，支持DataTable和Image两种输入
class Histogram : public NodeBase
{
public:
	Histogram() = default;

	void defineNode() override;
	void process() override;
	void onInputConnected(const QString& portName, DataType actualType) override;
};
