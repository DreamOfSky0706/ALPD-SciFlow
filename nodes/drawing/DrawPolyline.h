// nodes/drawing/DrawPolyline.h
#pragma once

#include "NodeBase.h"

class DrawPolyline : public NodeBase
{
public:
	DrawPolyline() = default;
	void defineNode() override;
	void process() override;
};
