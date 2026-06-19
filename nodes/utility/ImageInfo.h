// nodes/utility/ImageInfo.h
#pragma once

#include "NodeBase.h"

class ImageInfo : public NodeBase
{
public:
	ImageInfo() = default;
	void defineNode() override;
	void process() override;
};
