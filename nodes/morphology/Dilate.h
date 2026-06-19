// nodes/morphology/Dilate.h
#pragma once

#include "MorphBase.h"

// 膨胀操作
class Dilate : public MorphBase
{
protected:
	int morphOperation() const override;
};
