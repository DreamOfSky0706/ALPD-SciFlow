// nodes/channel/SetAlpha.h
#pragma once

#include "NodeBase.h"

// 设置Alpha通道
class SetAlpha : public NodeBase
{
public:
	SetAlpha() = default;

	void defineNode() override;
	void process() override;
};
