// nodes/data_proc/DataProfile.h
#pragma once
#include "NodeBase.h"

// 数据画像：自动生成数据统计摘要（对标专业数据软件）
class DataProfile : public NodeBase
{
public:
	DataProfile() = default;
	void defineNode() override;
	void process() override;
};
