// nodes/utility/DataConditionalRouter.h
#pragma once
#include "NodeBase.h"

// 通用条件路由：接受任意类型数据，按条件分发到true/false分支
class DataConditionalRouter : public NodeBase
{
public:
	DataConditionalRouter() = default;
	void defineNode() override;
	void process() override;
};
