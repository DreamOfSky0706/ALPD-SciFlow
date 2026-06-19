// nodes/morphology/TopHat.h
#pragma once

#include "MorphBase.h"

// 顶帽变换
class TopHat : public MorphBase
{
protected:
	int morphOperation() const override;
};
