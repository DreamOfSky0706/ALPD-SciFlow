// nodes/edge/FindContours.h
#pragma once

#include "NodeBase.h"

// 轮廓检测与绘制
class FindContours : public NodeBase
{
public:
	FindContours() = default;

	void defineNode() override;
	void process() override;
};
