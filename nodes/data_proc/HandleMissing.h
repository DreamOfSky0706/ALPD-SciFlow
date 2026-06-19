// nodes/data_proc/HandleMissing.h
#pragma once

#include "NodeBase.h"

// 缺失值处理
class HandleMissing : public NodeBase
{
public:
	HandleMissing() = default;

	void defineNode() override;
	void process() override;
};
