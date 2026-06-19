// nodes/color/WhiteBalance.h
#pragma once

#include "NodeBase.h"

// 色温/白平衡调整
class WhiteBalance : public NodeBase
{
public:
	WhiteBalance() = default;

	void defineNode() override;
	void process() override;
};
