// nodes/edge/Laplacian.h
#pragma once

#include "NodeBase.h"

// Laplacian算子
class Laplacian : public NodeBase
{
public:
	Laplacian() = default;

	void defineNode() override;
	void process() override;
};
