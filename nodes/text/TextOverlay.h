// nodes/text/TextOverlay.h
#pragma once

#include "NodeBase.h"

// 在图像上渲染文字
class TextOverlay : public NodeBase
{
public:
	TextOverlay() = default;

	void defineNode() override;
	void process() override;
};
