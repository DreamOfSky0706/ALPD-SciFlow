// nodes/filter/BilateralFilter.h
#pragma once

#include "NodeBase.h"

// 双边滤波，保边去噪
class BilateralFilter : public NodeBase
{
public:
	BilateralFilter() = default;

	void defineNode() override;
	void process() override;
};
