// nodes/color/Invert.h
#pragma once

#include "NodeBase.h"

// 反色
class Invert : public NodeBase
{
public:
	Invert() = default;

	void defineNode() override;
	void process() override;
};
