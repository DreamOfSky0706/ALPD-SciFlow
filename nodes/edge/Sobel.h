// nodes/edge/Sobel.h
#pragma once

#include "NodeBase.h"

// Sobel算子
class Sobel : public NodeBase
{
public:
	Sobel() = default;

	void defineNode() override;
	void process() override;
};
