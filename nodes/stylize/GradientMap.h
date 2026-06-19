// nodes/stylize/GradientMap.h
#pragma once

#include "NodeBase.h"

// 渐变映射：灰度值映射到渐变色带
class GradientMap : public NodeBase
{
public:
	GradientMap() = default;

	void defineNode() override;
	void process() override;
};
