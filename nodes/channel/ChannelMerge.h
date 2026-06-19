// nodes/channel/ChannelMerge.h
#pragma once

#include "NodeBase.h"

// 通道合并
class ChannelMerge : public NodeBase
{
public:
	ChannelMerge() = default;

	void defineNode() override;
	void process() override;
};
