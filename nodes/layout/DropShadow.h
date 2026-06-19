// nodes/layout/DropShadow.h
#pragma once

#include "NodeBase.h"

// 为图像添加投影效果
class DropShadow : public NodeBase
{
public:
	DropShadow() = default;

	void defineNode() override;
	void process() override;
};
