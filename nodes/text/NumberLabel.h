// nodes/text/NumberLabel.h
#pragma once

#include "NodeBase.h"

// 为图片添加编号标签
class NumberLabel : public NodeBase
{
public:
	NumberLabel() = default;

	void defineNode() override;
	void process() override;
};
