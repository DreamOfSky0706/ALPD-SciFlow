// nodes/text_io/NumericWrite.h
#pragma once
#include "NodeBase.h"

class NumericWrite : public NodeBase
{
public:
    NumericWrite() { defineNode(); }
    void defineNode() override;
    void process() override;
};
