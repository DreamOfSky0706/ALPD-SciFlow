// nodes/text/Annotation.h
#pragma once

#include "NodeBase.h"

// 箭头与标注：在图像上绘制箭头、矩形框、圆形框等
class Annotation : public NodeBase
{
public:
	Annotation() = default;

	void defineNode() override;
	void process() override;
};
