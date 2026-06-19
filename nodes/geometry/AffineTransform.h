// nodes/geometry/AffineTransform.h
#pragma once

#include "NodeBase.h"

// 仿射变换
class AffineTransform : public NodeBase
{
public:
	AffineTransform() = default;

	void defineNode() override;
	void process() override;
};
