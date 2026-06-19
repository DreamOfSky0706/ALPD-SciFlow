// nodes/data_proc/DataAggregate.h
#pragma once

#include "NodeBase.h"

// 数据聚合统计
class DataAggregate : public NodeBase
{
public:
	DataAggregate() = default;

	void defineNode() override;
	void process() override;
};
