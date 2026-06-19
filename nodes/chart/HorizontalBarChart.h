// nodes/chart/HorizontalBarChart.h
#pragma once

#include "NodeBase.h"

// 水平条形图
class HorizontalBarChart : public NodeBase
{
public:
	HorizontalBarChart() = default;

	void defineNode() override;
	void process() override;
};
