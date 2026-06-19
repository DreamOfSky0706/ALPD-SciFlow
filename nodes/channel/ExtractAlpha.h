// nodes/channel/ExtractAlpha.h
#pragma once

#include "NodeBase.h"

// 提取Alpha通道
class ExtractAlpha : public NodeBase
{
public:
	ExtractAlpha() = default;

	void defineNode() override;
	void process() override;
};
