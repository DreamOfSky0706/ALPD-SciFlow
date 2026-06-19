// nodes/chart/AreaChart.h
#pragma once

#include "NodeBase.h"

// 面积图
class AreaChart : public NodeBase
{
public:
	AreaChart() = default;

	void defineNode() override;
	void process() override;
};
