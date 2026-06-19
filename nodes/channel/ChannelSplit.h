// nodes/channel/ChannelSplit.h
#pragma once

#include "NodeBase.h"

// 通道拆分
class ChannelSplit : public NodeBase
{
public:
	ChannelSplit() = default;

	void defineNode() override;
	void process() override;
};
