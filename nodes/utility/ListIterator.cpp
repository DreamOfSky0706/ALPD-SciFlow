// nodes/utility/ListIterator.cpp
#include "ListIterator.h"
#include "Logger.h"
#include "WorkflowGraph.h"
#include "ExecutionEngine.h"
#include <algorithm>

void ListIterator::defineNode()
{
	addInputPort("input_list", DataType::ImageList);
	addInputPort("input_iteration_result", DataType::Image, false);
	addOutputPort("output_current", DataType::Image);
	addOutputPort("output_index", DataType::Numeric);
	addOutputPort("output_results", DataType::ImageList);
}

void ListIterator::process()
{
	auto listData = getInput("input_list");
	if (!listData || listData->isNull()) {
		reportError("输入列表为空"); return;
	}
	auto images = listData->toImageList();
	if (images.empty()) {
		setOutput("output_results", NodeData::createImageList({})); return;
	}

	// 将列表缓存到成员变量，供后续迭代读取
	m_imageList = images;
	m_currentIndex = 0;
	m_collected.clear();

	// 输出第一个元素启动迭代
	setOutput("output_current", NodeData::createImage(images[0]));
	setOutput("output_index", NodeData::createNumeric(0));
	Logger::instance().info(QString("列表遍历器：共%1张图").arg(images.size()));
}

void ListIterator::advanceIteration()
{
	m_currentIndex++;
	if (m_currentIndex < (int)m_imageList.size()) {
		// 收集上一轮结果
		auto result = getInput("input_iteration_result");
		if (result && !result->isNull())
			m_collected.push_back(result->toImage());
		clearOutputs();
		// 输出当前元素
		setOutput("output_current", NodeData::createImage(m_imageList[m_currentIndex]));
		setOutput("output_index", NodeData::createNumeric((double)m_currentIndex));
	} else {
		// 最后一轮：收集结果并输出
		auto result = getInput("input_iteration_result");
		if (result && !result->isNull())
			m_collected.push_back(result->toImage());
		clearOutputs();
		setOutput("output_results", NodeData::createImageList(m_collected));
		Logger::instance().success(QString("列表遍历完成：处理%1张图").arg(m_collected.size()));
	}
}

bool ListIterator::hasMore() const
{
	return m_currentIndex < (int)m_imageList.size();
}

int ListIterator::currentIndex() const
{
	return m_currentIndex;
}
