// nodes/color/SaturationAdjust.h
#pragma once

#include "NodeBase.h"

// 饱和度调整
class SaturationAdjust : public NodeBase
{
public:
	SaturationAdjust() = default;

	void defineNode() override;
	void process() override;
};
