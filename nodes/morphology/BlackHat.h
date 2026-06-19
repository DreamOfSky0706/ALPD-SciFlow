// nodes/morphology/BlackHat.h
#pragma once

#include "MorphBase.h"

// 黑帽变换
class BlackHat : public MorphBase
{
protected:
	int morphOperation() const override;
};
