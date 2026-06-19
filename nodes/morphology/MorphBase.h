// nodes/morphology/MorphBase.h
#pragma once

#include "NodeBase.h"

// 形态学操作的公共基类，子类只需指定操作类型
class MorphBase : public NodeBase
{
public:
	MorphBase() = default;

	void defineNode() override;
	void process() override;

protected:
	// 子类重写此方法返回对应的OpenCV形态学操作类型
	virtual int morphOperation() const = 0;
};
