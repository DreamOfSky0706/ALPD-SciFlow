// nodes/utility/DataQualityScore.h
#pragma once

#include "NodeBase.h"

class DataQualityScore : public NodeBase
{
public:
	DataQualityScore() = default;
	void defineNode() override;
	void process() override;
};
