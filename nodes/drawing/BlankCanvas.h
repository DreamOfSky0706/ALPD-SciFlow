// nodes/drawing/BlankCanvas.h
#pragma once

#include "NodeBase.h"

// 创建空白画布，作为绘制类工作流的起点
class BlankCanvas : public NodeBase
{
public:
	BlankCanvas() = default;

	void defineNode() override;
	void process() override;
};
