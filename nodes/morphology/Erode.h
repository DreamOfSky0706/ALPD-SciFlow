// nodes/morphology/Erode.h
#pragma once

#include "MorphBase.h"

// 腐蚀操作
class Erode : public MorphBase
{
protected:
	int morphOperation() const override;
};
