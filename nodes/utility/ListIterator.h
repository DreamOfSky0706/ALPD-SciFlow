// nodes/utility/ListIterator.h
#pragma once

#include "NodeBase.h"
#include <opencv2/core.hpp>
#include <vector>

class ListIterator : public NodeBase
{
public:
	ListIterator() = default;
	void defineNode() override;
	void process() override;

	bool isIterator() const override { return true; }
	void advanceIteration() override;
	bool hasMore() const override;
	int  currentIndex() const override;

private:
	std::vector<cv::Mat> m_imageList;
	std::vector<cv::Mat> m_collected;
	int m_currentIndex = 0;
};
