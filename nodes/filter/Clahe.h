// nodes/filter/Clahe.h
#pragma once
#include "NodeBase.h"

// CLAHE 自适应直方图均衡化（对标专业图像处理软件）
class Clahe : public NodeBase
{
public:
	Clahe() = default;
	void defineNode() override;
	void process() override;
};
