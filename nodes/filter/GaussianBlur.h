// nodes/filter/GaussianBlur.h
#pragma once

#include "NodeBase.h"

// 高斯模糊
class GaussianBlur : public NodeBase
{
public:
	GaussianBlur() = default;

	void defineNode() override;
	void process() override;
};
