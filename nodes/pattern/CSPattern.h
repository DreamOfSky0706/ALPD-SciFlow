// nodes/pattern/CSPattern.h
#pragma once
#include "PatternBase.h"
class CSPattern : public PatternBase
{
public: CSPattern() = default; void defineNode() override; void process() override;
};
