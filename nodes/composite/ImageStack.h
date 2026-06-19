// nodes/composite/ImageStack.h
#pragma once

#include "NodeBase.h"

// 将两张图上下或左右拼接
class ImageStack : public NodeBase
{
public:
	ImageStack() = default;

	void defineNode() override;
	void process() override;
};
