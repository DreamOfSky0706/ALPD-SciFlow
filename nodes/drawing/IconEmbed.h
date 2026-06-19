// nodes/drawing/IconEmbed.h
#pragma once

#include "NodeBase.h"

// 在图像上嵌入图标
class IconEmbed : public NodeBase
{
public:
	IconEmbed() = default;
	void defineNode() override;
	void process() override;
};
