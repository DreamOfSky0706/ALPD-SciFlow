// nodes/text_io/TextFileWrite.h
#pragma once
#include "NodeBase.h"

class TextFileWrite : public NodeBase
{
public:
    TextFileWrite() { defineNode(); }
    void defineNode() override;
    void process() override;
};
