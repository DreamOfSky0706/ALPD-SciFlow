// nodes/image_io/BatchImageRead.h
#pragma once

#include "NodeBase.h"

// 批量读取目录下的图片文件
class BatchImageRead : public NodeBase
{
public:
	BatchImageRead() = default;

	void defineNode() override;
	void process() override;
};
