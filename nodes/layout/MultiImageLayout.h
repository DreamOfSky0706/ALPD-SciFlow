// nodes/layout/MultiImageLayout.h
#pragma once

#include "NodeBase.h"

// 多图排版：支持m×n网格、格子启用/禁用、格子合并、动态端口
class MultiImageLayout : public NodeBase
{
public:
	MultiImageLayout() = default;

	void defineNode() override;
	void process() override;
	void onParamChanged(const QString& paramName) override;
	void rebuildPorts() override;

private:
	int countEnabledGroups() const;
};
