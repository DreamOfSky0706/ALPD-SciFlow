// nodes/data_proc/RemoveDuplicates.h
#pragma once

#include "NodeBase.h"

// 去重
class RemoveDuplicates : public NodeBase
{
public:
	RemoveDuplicates() = default;

	void defineNode() override;
	void process() override;
};
