// nodes/composite/Blend.h
#pragma once

#include "NodeBase.h"

// 图像混合（多种混合模式）
class Blend : public NodeBase
{
public:
	Blend() = default;

	void defineNode() override;
	void process() override;

private:
	// 对单个通道值应用混合公式，a为底层，b为顶层，均归一化到[0,1]
	double blendPixel(double a, double b, const QString& mode) const;
};
