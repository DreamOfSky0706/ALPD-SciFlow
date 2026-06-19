// nodes/utility/ExternalAPICall.h
#pragma once

#include "NodeBase.h"

// 接口预留，暂不实现
class ExternalAPICall : public NodeBase
{
public:
	ExternalAPICall() = default;
	void defineNode() override;
	void process() override;
};
