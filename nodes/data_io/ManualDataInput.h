// nodes/data_io/ManualDataInput.h
#pragma once

#include "NodeBase.h"

// 手动数据输入：用户在表格编辑器中直接编辑数据
class ManualDataInput : public NodeBase
{
public:
	ManualDataInput() = default;

	void defineNode() override;
	void process() override;
};
