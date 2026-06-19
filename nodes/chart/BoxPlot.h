// nodes/chart/BoxPlot.h
#pragma once

#include "NodeBase.h"

// 箱线图
class BoxPlot : public NodeBase
{
public:
	BoxPlot() = default;

	void defineNode() override;
	void process() override;
};
