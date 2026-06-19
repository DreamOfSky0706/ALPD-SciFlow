// nodes/filter/Sharpen.h
#pragma once

#include "NodeBase.h"

// USM锐化（非锐化蒙版）
class Sharpen : public NodeBase
{
public:
	Sharpen() = default;

	void defineNode() override;
	void process() override;
};
