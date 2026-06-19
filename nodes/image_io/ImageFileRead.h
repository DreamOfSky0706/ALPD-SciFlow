// nodes/image_io/ImageFileRead.h
#pragma once

#include "NodeBase.h"

// 从磁盘读取一张图片文件
class ImageFileRead : public NodeBase
{
public:
	ImageFileRead() = default;

	void defineNode() override;
	void process() override;
};
