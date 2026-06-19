// nodes/chart/BarChart.h
#pragma once

#include "NodeBase.h"

// 柱状图（分组/堆叠）
class BarChart : public NodeBase
{
public:
	BarChart() = default;

	void defineNode() override;
	void process() override;
};
