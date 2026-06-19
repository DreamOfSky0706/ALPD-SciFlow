// nodes/data_io/JSONDataRead.h
#pragma once

#include "NodeBase.h"

// JSON数据读取
class JSONDataRead : public NodeBase
{
public:
	JSONDataRead() = default;

	void defineNode() override;
	void process() override;
};
