// nodes/composite/ImageGridStitch.h
#pragma once

#include "NodeBase.h"

// 将ImageList中的图像按网格排列成一张大图
class ImageGridStitch : public NodeBase
{
public:
	ImageGridStitch() = default;

	void defineNode() override;
	void process() override;
};
