// nodes/channel/ConvertToGray.h
#pragma once

#include "NodeBase.h"

// 转灰度
class ConvertToGray : public NodeBase
{
public:
	ConvertToGray() = default;

	void defineNode() override;
	void process() override;
};
