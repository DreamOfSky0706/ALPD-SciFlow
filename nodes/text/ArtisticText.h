// nodes/text/ArtisticText.h
#pragma once

#include "NodeBase.h"

// 艺术字生成：描边、阴影、渐变色填充
class ArtisticText : public NodeBase
{
public:
	ArtisticText() = default;

	void defineNode() override;
	void process() override;
};
