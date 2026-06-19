// nodes/drawing/DrawRoundedRect.h
#pragma once

#include "NodeBase.h"

// 圆角矩形绘制
class DrawRoundedRect : public NodeBase
{
public:
	DrawRoundedRect() = default;

	void defineNode() override;
	void process() override;
};
