// nodes/color/LightnessAdjust.h
#pragma once

#include "NodeBase.h"

// 明度调整
class LightnessAdjust : public NodeBase
{
public:
	LightnessAdjust() = default;

	void defineNode() override;
	void process() override;
};
