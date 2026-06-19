// nodes/pattern/ChemistryPattern.h
#pragma once
#include "PatternBase.h"
class ChemistryPattern : public PatternBase
{
public: ChemistryPattern() = default; void defineNode() override; void process() override;
};
