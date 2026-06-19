// nodes/data_proc/DataNormalize.h
#pragma once

#include "NodeBase.h"

// 数据归一化
class DataNormalize : public NodeBase
{
public:
	DataNormalize() = default;

	void defineNode() override;
	void process() override;
};
