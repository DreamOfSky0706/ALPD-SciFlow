// nodes/stylize/SketchEffect.h
#pragma once

#include "NodeBase.h"

// 素描效果
class SketchEffect : public NodeBase
{
public:
	SketchEffect() = default;

	void defineNode() override;
	void process() override;
};
