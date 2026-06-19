// nodes/composite/TextWatermark.h
#pragma once

#include "NodeBase.h"

// 文字水印叠加
class TextWatermark : public NodeBase
{
public:
	TextWatermark() = default;

	void defineNode() override;
	void process() override;
};
