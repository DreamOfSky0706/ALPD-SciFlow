// nodes/chart/RadarChart.h
#pragma once

#include "NodeBase.h"

// 雷达图，使用Qt Charts极坐标
class RadarChart : public NodeBase
{
public:
	RadarChart() = default;

	void defineNode() override;
	void process() override;
};
