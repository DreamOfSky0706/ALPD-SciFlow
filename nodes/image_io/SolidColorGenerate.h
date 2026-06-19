// nodes/image_io/SolidColorGenerate.h
#pragma once

#include "NodeBase.h"

// 生成纯色图像
class SolidColorGenerate : public NodeBase
{
public:
	SolidColorGenerate() = default;

	void defineNode() override;
	void process() override;
};
