// nodes/data_io/CSVRead.h
#pragma once

#include "NodeBase.h"

// CSV文件读取
class CSVRead : public NodeBase
{
public:
	CSVRead() = default;

	void defineNode() override;
	void process() override;
};
