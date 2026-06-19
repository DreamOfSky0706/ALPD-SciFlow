// nodes/layout/ImagePreview.h
#pragma once

#include "NodeBase.h"

// 图像预览显示节点（在GUI的预览面板中显示）
class ImagePreview : public NodeBase
{
public:
	ImagePreview() = default;

	void defineNode() override;
	void process() override;
};
