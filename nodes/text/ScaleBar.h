// nodes/text/ScaleBar.h
#pragma once

#include "NodeBase.h"

// 在科研图片上添加比例尺
class ScaleBar : public NodeBase
{
public:
	ScaleBar() = default;

	void defineNode() override;
	void process() override;
};
