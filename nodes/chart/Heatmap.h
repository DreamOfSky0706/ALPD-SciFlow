// nodes/chart/Heatmap.h
#pragma once

#include "NodeBase.h"

// 热力图
class Heatmap : public NodeBase
{
public:
	Heatmap() = default;

	void defineNode() override;
	void process() override;
};
