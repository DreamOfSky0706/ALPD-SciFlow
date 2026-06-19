// nodes/geometry/Resize.h
#pragma once

#include "NodeBase.h"

// 缩放图像
class Resize : public NodeBase
{
public:
	Resize() = default;

	void defineNode() override;
	void process() override;
};
