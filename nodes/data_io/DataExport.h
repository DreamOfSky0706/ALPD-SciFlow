// nodes/data_io/DataExport.h
#pragma once

#include "NodeBase.h"

// 数据导出为CSV或JSON
class DataExport : public NodeBase
{
public:
	DataExport() = default;

	void defineNode() override;
	void process() override;
};
