// nodes/chart/PieChart.h
#pragma once

#include "NodeBase.h"

// 饼图，使用Qt Charts
class PieChart : public NodeBase
{
public:
	PieChart() = default;

	void defineNode() override;
	void process() override;
};
