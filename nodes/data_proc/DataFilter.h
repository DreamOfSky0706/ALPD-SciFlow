// nodes/data_proc/DataFilter.h
#pragma once

#include "NodeBase.h"

// 按条件筛选行
class DataFilter : public NodeBase
{
public:
	DataFilter() = default;

	void defineNode() override;
	void process() override;
};
