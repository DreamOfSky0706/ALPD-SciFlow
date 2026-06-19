// nodes/morphology/MorphOpen.h
#pragma once

#include "MorphBase.h"

// 开运算
class MorphOpen : public MorphBase
{
protected:
	int morphOperation() const override;
};
