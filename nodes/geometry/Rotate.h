// nodes/geometry/Rotate.h
#pragma once

#include "NodeBase.h"

// 旋转图像
class Rotate : public NodeBase
{
public:
	Rotate() = default;

	void defineNode() override;
	void process() override;
};
