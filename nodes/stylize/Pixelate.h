// nodes/stylize/Pixelate.h
#pragma once

#include "NodeBase.h"

// 像素化/马赛克
class Pixelate : public NodeBase
{
public:
	Pixelate() = default;

	void defineNode() override;
	void process() override;
};
