// nodes/color/Curves.h
#pragma once

#include "NodeBase.h"

// 曲线调整：通过控制点样条插值构建查找表
class Curves : public NodeBase
{
public:
	Curves() = default;

	void defineNode() override;
	void process() override;

private:
	// 对控制点做单调三次样条插值，生成256项查找表
	void buildLUT(const QVariantList& points, uchar* lut) const;
};
