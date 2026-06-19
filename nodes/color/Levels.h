// nodes/color/Levels.h
#pragma once

#include "NodeBase.h"

// 色阶调整
class Levels : public NodeBase
{
public:
	Levels() = default;

	void defineNode() override;
	void process() override;
};
