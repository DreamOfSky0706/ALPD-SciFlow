// nodes/edge/AdaptiveThreshold.h
#pragma once

#include "NodeBase.h"

// 自适应阈值分割
class AdaptiveThreshold : public NodeBase
{
public:
	AdaptiveThreshold() = default;

	void defineNode() override;
	void process() override;
};
