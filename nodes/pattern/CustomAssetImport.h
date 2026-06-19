// nodes/pattern/CustomAssetImport.h
#pragma once
#include "NodeBase.h"

class CustomAssetImport : public NodeBase
{
public:
	CustomAssetImport() = default;
	void defineNode() override;
	void process() override;
};
