// nodes/utility/ImageHistogramStats.h
#pragma once

#include "NodeBase.h"

class ImageHistogramStats : public NodeBase
{
public:
	ImageHistogramStats() = default;
	void defineNode() override;
	void process() override;
};
