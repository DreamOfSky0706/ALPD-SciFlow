// nodes/text_io/TextFileRead.h
#pragma once
#include "NodeBase.h"

class TextFileRead : public NodeBase
{
public:
    TextFileRead() { defineNode(); }
    void defineNode() override;
    void process() override;
};
