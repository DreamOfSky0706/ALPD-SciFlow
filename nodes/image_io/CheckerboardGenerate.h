// nodes/image_io/CheckerboardGenerate.h
#pragma once

#include "NodeBase.h"

// 生成棋盘格图案
class CheckerboardGenerate : public NodeBase
{
public:
	CheckerboardGenerate() = default;

	void defineNode() override;
	void process() override;
};
