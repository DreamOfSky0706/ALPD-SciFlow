// nodes/color/ColorBalance.h
#pragma once

#include "NodeBase.h"

// 色彩平衡：分别调整阴影/中间调/高光的RGB偏移
class ColorBalance : public NodeBase
{
public:
	ColorBalance() = default;

	void defineNode() override;
	void process() override;
};
