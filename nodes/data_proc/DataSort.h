// nodes/data_proc/DataSort.h
#pragma once

#include "NodeBase.h"

// 数据排序
class DataSort : public NodeBase
{
public:
	DataSort() = default;

	void defineNode() override;
	void process() override;
};
