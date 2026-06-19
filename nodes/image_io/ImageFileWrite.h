// nodes/image_io/ImageFileWrite.h
#pragma once

#include "NodeBase.h"

// 将图像写入磁盘文件
class ImageFileWrite : public NodeBase
{
public:
	ImageFileWrite() = default;

	void defineNode() override;
	void process() override;
};
