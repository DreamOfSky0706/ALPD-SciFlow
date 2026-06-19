// nodes/drawing/DrawEllipse.h
#pragma once

#include "NodeBase.h"

class DrawEllipse : public NodeBase
{
public:
	DrawEllipse() = default;
	void defineNode() override;
	void process() override;
};
