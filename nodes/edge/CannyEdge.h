// nodes/edge/CannyEdge.h
#pragma once

#include "NodeBase.h"

// Canny边缘检测
class CannyEdge : public NodeBase
{
public:
	CannyEdge() = default;

	void defineNode() override;
	void process() override;
};
