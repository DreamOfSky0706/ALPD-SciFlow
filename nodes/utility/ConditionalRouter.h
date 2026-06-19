// nodes/utility/ConditionalRouter.h
#pragma once

#include "NodeBase.h"

class ConditionalRouter : public NodeBase
{
public:
	ConditionalRouter() = default;
	void defineNode() override;
	void process() override;
};
