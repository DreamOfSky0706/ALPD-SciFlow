// nodes/channel/ConvertToLab.h
#pragma once

#include "NodeBase.h"

// 转Lab色彩空间
class ConvertToLab : public NodeBase
{
public:
	ConvertToLab() = default;

	void defineNode() override;
	void process() override;
};
