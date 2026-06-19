// nodes/data_io/ExcelRead.h
#pragma once

#include "NodeBase.h"

// Excel(.xlsx)文件读取
class ExcelRead : public NodeBase
{
public:
	ExcelRead() = default;

	void defineNode() override;
	void process() override;
};
