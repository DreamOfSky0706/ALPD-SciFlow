// nodes/geometry/Crop.h
#pragma once

#include "NodeBase.h"

// 裁剪图像矩形区域
class Crop : public NodeBase
{
public:
	Crop() = default;

	void defineNode() override;
	void process() override;
};
