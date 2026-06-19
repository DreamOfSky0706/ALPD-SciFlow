// nodes/data_proc/DataPivot.h
#pragma once
#include "NodeBase.h"

// 数据透视表（对标Excel透视表功能）
class DataPivot : public NodeBase
{
public:
	DataPivot() = default;
	void defineNode() override;
	void process() override;
};
