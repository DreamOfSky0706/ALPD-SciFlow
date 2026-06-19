// nodes/layout/ImageDiff.h
#pragma once
#include "NodeBase.h"

// 图像差异可视化（科研对比常用）
class ImageDiff : public NodeBase
{
public:
	ImageDiff() = default;
	void defineNode() override;
	void process() override;
};
