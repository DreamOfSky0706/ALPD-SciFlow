// nodes/channel/ConvertToHSV.h
#pragma once

#include "NodeBase.h"

// 转HSV色彩空间
class ConvertToHSV : public NodeBase
{
public:
	ConvertToHSV() = default;

	void defineNode() override;
	void process() override;
};
