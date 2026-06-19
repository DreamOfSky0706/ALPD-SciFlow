// nodes/drawing/DrawRectangle.h
#pragma once

#include "NodeBase.h"

// 在图像上绘制矩形
class DrawRectangle : public NodeBase
{
public:
	DrawRectangle() = default;

	void defineNode() override;
	void process() override;
};
