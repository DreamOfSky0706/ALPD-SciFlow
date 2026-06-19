// nodes/layout/CanvasExtend.h
#pragma once

#include "NodeBase.h"

// 扩展画布边界
class CanvasExtend : public NodeBase
{
public:
	CanvasExtend() = default;

	void defineNode() override;
	void process() override;
};
