// nodes/image_io/GradientGenerate.h
#pragma once

#include "NodeBase.h"

// 生成线性或径向渐变图像
class GradientGenerate : public NodeBase
{
public:
	GradientGenerate() = default;

	void defineNode() override;
	void process() override;
};
