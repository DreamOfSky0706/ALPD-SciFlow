// nodes/drawing/DrawParallelogram.h
#pragma once

#include "NodeBase.h"

class DrawParallelogram : public NodeBase
{
public:
	DrawParallelogram() = default;
	void defineNode() override;
	void process() override;
};
