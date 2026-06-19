// nodes/chart/DoughnutChart.h
#pragma once

#include "NodeBase.h"

// 环形图
class DoughnutChart : public NodeBase
{
public:
	DoughnutChart() = default;

	void defineNode() override;
	void process() override;
};
