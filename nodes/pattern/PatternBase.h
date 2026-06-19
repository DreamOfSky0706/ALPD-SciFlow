// nodes/pattern/PatternBase.h
#pragma once

#include "NodeBase.h"

// 图案节点的公共参数和工具
class PatternBase : public NodeBase
{
public:
	PatternBase() = default;

protected:
	void addCommonParams();
	// 从公共参数中获取画布和颜色信息
	void getCommonValues(int& w, int& h, QColor& primary, QColor& secondary, QColor& bg, double& density, int& seed) const;
};
