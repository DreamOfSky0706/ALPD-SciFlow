// nodes/drawing/FlowchartElement.h
#pragma once

#include "NodeBase.h"

// 在底图上绘制单个流程图元素（形状+文字）
class FlowchartElement : public NodeBase
{
public:
	FlowchartElement() = default;
	void defineNode() override;
	void process() override;
};
