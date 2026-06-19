// nodes/edge/OtsuThreshold.h
#pragma once

#include "NodeBase.h"

// Otsu自动阈值分割
class OtsuThreshold : public NodeBase
{
public:
	OtsuThreshold() = default;

	void defineNode() override;
	void process() override;
};
