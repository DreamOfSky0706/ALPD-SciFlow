// nodes/utility/ExternalScript.h
#pragma once

#include "NodeBase.h"

class ExternalScript : public NodeBase
{
public:
	ExternalScript() = default;
	void defineNode() override;
	void process() override;
};
