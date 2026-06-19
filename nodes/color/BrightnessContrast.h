// nodes/color/BrightnessContrast.h
#pragma once

#include "NodeBase.h"

// 亮度和对比度调整
class BrightnessContrast : public NodeBase
{
public:
	BrightnessContrast() = default;

	void defineNode() override;
	void process() override;
};
