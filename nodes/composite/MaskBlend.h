// nodes/composite/MaskBlend.h
#pragma once

#include "NodeBase.h"

// 蒙版混合
class MaskBlend : public NodeBase
{
public:
	MaskBlend() = default;

	void defineNode() override;
	void process() override;
};
