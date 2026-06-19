// nodes/stylize/EmbossEffect.h
#pragma once
#include "NodeBase.h"
class EmbossEffect : public NodeBase {
public: EmbossEffect() = default; void defineNode() override; void process() override;
};
