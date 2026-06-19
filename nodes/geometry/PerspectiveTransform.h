// nodes/geometry/PerspectiveTransform.h
#pragma once

#include "NodeBase.h"

// 透视变换
class PerspectiveTransform : public NodeBase
{
public:
	PerspectiveTransform() = default;

	void defineNode() override;
	void process() override;
};
