// nodes/utility/TableInfo.h
#pragma once
#include "NodeBase.h"
class TableInfo : public NodeBase {
public: TableInfo() = default; void defineNode() override; void process() override;
};
