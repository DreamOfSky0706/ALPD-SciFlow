// nodes/edge/GlobalThreshold.h
#pragma once

#include "NodeBase.h"

// 全局阈值分割
class GlobalThreshold : public NodeBase
{
public:
	GlobalThreshold() = default;

	void defineNode() override;
	void process() override;
};
