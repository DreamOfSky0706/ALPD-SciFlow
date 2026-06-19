// nodes/utility/TextInfo.h
#pragma once
#include "NodeBase.h"
class TextInfo : public NodeBase {
public: TextInfo() = default; void defineNode() override; void process() override;
};
