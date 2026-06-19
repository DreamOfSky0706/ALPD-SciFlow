// nodes/stylize/VintageEffect.h
#pragma once

#include "NodeBase.h"

// 复古/老照片效果
class VintageEffect : public NodeBase
{
public:
	VintageEffect() = default;

	void defineNode() override;
	void process() override;
};
