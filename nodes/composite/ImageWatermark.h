// nodes/composite/ImageWatermark.h
#pragma once

#include "NodeBase.h"

// 图片水印叠加
class ImageWatermark : public NodeBase
{
public:
	ImageWatermark() = default;

	void defineNode() override;
	void process() override;
};
