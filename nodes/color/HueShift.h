// nodes/color/HueShift.h
#pragma once

#include "NodeBase.h"

// 色相偏移
class HueShift : public NodeBase
{
public:
	HueShift() = default;

	void defineNode() override;
	void process() override;
};
