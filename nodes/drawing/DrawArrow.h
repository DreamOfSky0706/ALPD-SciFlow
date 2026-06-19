// nodes/drawing/DrawArrow.h
#pragma once

#include "NodeBase.h"

class DrawArrow : public NodeBase
{
public:
	DrawArrow() = default;
	void defineNode() override;
	void process() override;
};
