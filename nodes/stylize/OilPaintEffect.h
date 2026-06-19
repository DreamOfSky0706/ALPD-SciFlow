// nodes/stylize/OilPaintEffect.h
#pragma once

#include "NodeBase.h"

// 油画效果
class OilPaintEffect : public NodeBase
{
public:
	OilPaintEffect() = default;

	void defineNode() override;
	void process() override;
};
