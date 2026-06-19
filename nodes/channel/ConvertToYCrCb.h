// nodes/channel/ConvertToYCrCb.h
#pragma once

#include "NodeBase.h"

// 转YCrCb色彩空间
class ConvertToYCrCb : public NodeBase
{
public:
	ConvertToYCrCb() = default;

	void defineNode() override;
	void process() override;
};
