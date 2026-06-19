// nodes/pattern/GenericPattern.h
#pragma once

#include "PatternBase.h"

class GenericPattern : public PatternBase
{
public:
	GenericPattern() = default;
	void defineNode() override;
	void process() override;
};
