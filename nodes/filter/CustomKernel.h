// nodes/filter/CustomKernel.h
#pragma once

#include "NodeBase.h"

// 自定义卷积核
class CustomKernel : public NodeBase
{
public:
	CustomKernel() = default;

	void defineNode() override;
	void process() override;
};
