// nodes/drawing/DrawDiamond.h
#pragma once

#include "NodeBase.h"

class DrawDiamond : public NodeBase
{
public:
	DrawDiamond() = default;
	void defineNode() override;
	void process() override;
};
