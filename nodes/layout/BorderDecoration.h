// nodes/layout/BorderDecoration.h
#pragma once

#include "NodeBase.h"

// 边框与装饰
class BorderDecoration : public NodeBase
{
public:
	BorderDecoration() = default;

	void defineNode() override;
	void process() override;
};
