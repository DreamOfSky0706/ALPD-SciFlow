// nodes/filter/MotionBlur.h
#pragma once

#include "NodeBase.h"

// 运动模糊
class MotionBlur : public NodeBase
{
public:
	MotionBlur() = default;

	void defineNode() override;
	void process() override;
};
