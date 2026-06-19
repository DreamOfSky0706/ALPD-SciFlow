// nodes/morphology/MorphClose.h
#pragma once

#include "MorphBase.h"

// 闭运算
class MorphClose : public MorphBase
{
protected:
	int morphOperation() const override;
};
