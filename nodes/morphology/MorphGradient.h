// nodes/morphology/MorphGradient.h
#pragma once

#include "MorphBase.h"

// 形态学梯度（膨胀图与腐蚀图的差值）
class MorphGradient : public MorphBase
{
protected:
	int morphOperation() const override;
};
