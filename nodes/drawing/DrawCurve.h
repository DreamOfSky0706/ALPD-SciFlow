// nodes/drawing/DrawCurve.h
#pragma once

#include "NodeBase.h"

class DrawCurve : public NodeBase
{
public:
	DrawCurve() = default;
	void defineNode() override;
	void process() override;
};
