// nodes/pattern/BioPattern.h
#pragma once
#include "PatternBase.h"
class BioPattern : public PatternBase
{
public: BioPattern() = default; void defineNode() override; void process() override;
};
