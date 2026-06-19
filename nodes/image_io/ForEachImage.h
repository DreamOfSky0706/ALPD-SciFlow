// nodes/image_io/ForEachImage.h
#pragma once
#include "NodeBase.h"

// 批次循环处理容器：对ImageList中每张图依次执行选定操作后输出
class ForEachImage : public NodeBase
{
public:
	ForEachImage() = default;
	void defineNode() override;
	void process() override;
};
