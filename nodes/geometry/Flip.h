// nodes/geometry/Flip.h
#pragma once

#include "NodeBase.h"

// 翻转图像
class Flip : public NodeBase
{
public:
	Flip() = default;

	void defineNode() override;
	void process() override;
};
