// nodes/composite/OpacityAdjust.h
#pragma once

#include "NodeBase.h"

// 透明度调整
class OpacityAdjust : public NodeBase
{
public:
	OpacityAdjust() = default;

	void defineNode() override;
	void process() override;
};
