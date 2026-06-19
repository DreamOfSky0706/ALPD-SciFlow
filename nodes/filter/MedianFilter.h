// nodes/filter/MedianFilter.h
#pragma once

#include "NodeBase.h"

// 中值滤波
class MedianFilter : public NodeBase
{
public:
	MedianFilter() = default;

	void defineNode() override;
	void process() override;
};
