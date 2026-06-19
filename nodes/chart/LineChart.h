// nodes/chart/LineChart.h
#pragma once

#include "NodeBase.h"

// 折线图
class LineChart : public NodeBase
{
public:
	LineChart() = default;

	void defineNode() override;
	void process() override;
};
