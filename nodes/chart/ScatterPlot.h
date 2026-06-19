// nodes/chart/ScatterPlot.h
#pragma once

#include "NodeBase.h"

// 散点图
class ScatterPlot : public NodeBase
{
public:
	ScatterPlot() = default;

	void defineNode() override;
	void process() override;
};
