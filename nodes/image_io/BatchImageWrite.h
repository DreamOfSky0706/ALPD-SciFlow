// nodes/image_io/BatchImageWrite.h
#pragma once

#include "NodeBase.h"

// 批量导出图像列表
class BatchImageWrite : public NodeBase
{
public:
	BatchImageWrite() = default;

	void defineNode() override;
	void process() override;
};
