// nodes/text/LegendOverlay.h
#pragma once

#include "NodeBase.h"

// 在图像上绘制颜色图例
class LegendOverlay : public NodeBase
{
public:
	LegendOverlay() = default;

	void defineNode() override;
	void process() override;
};
