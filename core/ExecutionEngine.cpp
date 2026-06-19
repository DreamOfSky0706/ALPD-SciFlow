// core/ExecutionEngine.cpp
#include "ExecutionEngine.h"
#include "WorkflowGraph.h"
#include "NodeBase.h"
#include "Connection.h"
#include "Logger.h"
#include "Exceptions.h"
#include <opencv2/core.hpp>
#include <QElapsedTimer>

ExecutionEngine::ExecutionEngine(WorkflowGraph* graph, QObject* parent)
	: QObject(parent)
	, m_graph(graph)
{
}

void ExecutionEngine::executeAll()
{
	Logger::instance().info("══════════════ 开始执行 ══════════════");
	emit m_graph->executionStarted();

	QVector<NodeBase*> sorted = m_graph->topologicalSort();
	executeNodeList(sorted);
}

void ExecutionEngine::executePartial(const QString& targetNodeId, int direction)
{
	auto dir = static_cast<WorkflowGraph::PartialDirection>(direction);
	QString dirName = (dir == WorkflowGraph::Upstream) ? "上游" : "下游";
	Logger::instance().info(QString("部分执行：%1方向，目标节点[%2]")
							.arg(dirName, targetNodeId));
	emit m_graph->executionStarted();

	QVector<NodeBase*> sorted = m_graph->topologicalSortPartial(targetNodeId, dir);
	executeNodeList(sorted);
}

void ExecutionEngine::executeNodeList(const QVector<NodeBase*>& nodes)
{
	QElapsedTimer totalTimer;
	totalTimer.start();

	int total = nodes.size();
	int successCount = 0;
	int failCount = 0;
	int skipCount = 0;

	Logger::instance().info("══════════════════════════════════════");

	for (int i = 0; i < total; ++i)
	{
		NodeBase* node = nodes[i];
		emit m_graph->executionProgress(i + 1, total, node->displayName());

		// 如果缓存有效且非强制执行，跳过
		if (node->isCacheValid())
		{
			Logger::instance().info(
				QString("节点[%1]缓存有效，跳过执行").arg(node->displayName()));
			skipCount++;
			continue;
		}

		// 注入输入数据
		feedInputsForNode(node);
		// 检查是否有required端口收到null（上游条件路由截断）
		bool shouldSkip = false;
		for (const auto& p : node->inputPorts()) {
			if (p.required) {
				auto conn = m_graph->findConnectionToInput(node->id(), p.name);
				if (conn) {
					auto src = m_graph->nodeById(conn->sourceNodeId());
					if (src) {
						auto d = src->getOutput(conn->sourcePortName());
						if (!d || d->isNull()) { shouldSkip = true; break; }
					}
				}
			}
		}
		if (shouldSkip) {
			Logger::instance().info(QString("节点[%1]跳过：上游未产生输出").arg(node->displayName()));
			skipCount++;
			continue;
		}

		// 执行
		bool ok = executeSingleNode(node);
		if (ok) {
			successCount++;
			// 迭代器节点特殊处理
			if (node->isIterator()) {
				QString iterId = node->id();
				while (node->hasMore()) {
					// 找拓扑排序中迭代器之后的下游节点
					QVector<NodeBase*> allSorted = m_graph->topologicalSort();
					QVector<NodeBase*> subNodes;
					bool pastIter = false;
					for (auto* n : allSorted) {
						if (n->id() == iterId) { pastIter = true; continue; }
						if (!pastIter) continue;
						// 若是另一个迭代器，停止
						if (n->isIterator()) break;
						subNodes.append(n);
					}
					// 执行子图（跳过回连到迭代器的节点）
					NodeBase* lastResult = nullptr;
					for (auto* sn : subNodes) {
						feedInputsForNode(sn);
						if (executeSingleNode(sn)) lastResult = sn;
					}
					// 反馈结果到迭代器
					if (lastResult) {
						auto outs = lastResult->outputPorts();
						if (!outs.isEmpty()) {
							auto d = lastResult->getOutput(outs.first().name);
							node->feedInput("input_iteration_result", d);
						}
					}
					node->advanceIteration();
				}
			}
		} else {
			failCount++;
		}
	}

	qint64 elapsed = totalTimer.elapsed();
	Logger::instance().success(
		QString("执行完成，耗时%1ms，成功%2个，失败%3个，跳过%4个")
		.arg(elapsed)
		.arg(successCount)
		.arg(failCount)
		.arg(skipCount));

	m_lastFailCount = failCount;
	emit m_graph->executionFinished(successCount, failCount, skipCount);
}

void ExecutionEngine::feedInputsForNode(NodeBase* node)
{
	const auto& inputPorts = node->inputPorts();
	for (const auto& port : inputPorts)
	{
		// 查找连接到此输入端口的连线
		Connection* conn = m_graph->findConnectionToInput(node->id(), port.name);
		if (!conn)
		{
			// 没有连线，输入为空
			node->feedInput(port.name, nullptr);
			continue;
		}

		// 从上游节点的输出端口获取数据
		NodeBase* srcNode = m_graph->nodeById(conn->sourceNodeId());
		if (!srcNode)
		{
			node->feedInput(port.name, nullptr);
			continue;
		}

		auto data = srcNode->getOutput(conn->sourcePortName());
		node->feedInput(port.name, data);
	}
}

bool ExecutionEngine::executeSingleNode(NodeBase* node)
{
	// 校验
	QString errorMsg;
	if (!node->validate(errorMsg))
	{
		Logger::instance().error(
			QString("节点[%1]校验失败：%2").arg(node->displayName(), errorMsg));
		return false;
	}

	// 执行
	try
	{
		node->clearOutputs();
		node->process();
		Logger::instance().info(
			QString("节点[%1]执行完成").arg(node->displayName()));
		return true;
	}
	catch (const NodeProcessException& e)
	{
		Logger::instance().error(
			QString("节点[%1]执行错误：%2").arg(node->displayName(), e.message()));
	}
	catch (const cv::Exception& e)
	{
		Logger::instance().error(
			QString("节点[%1]OpenCV错误：%2").arg(node->displayName(), e.what()));
	}
	catch (const std::bad_alloc&)
	{
		Logger::instance().error(
			QString("节点[%1]内存分配失败：图像过大或系统内存不足")
			.arg(node->displayName()));
	}
	catch (const std::exception& e)
	{
		Logger::instance().error(
			QString("节点[%1]未预期错误：%2").arg(node->displayName(), e.what()));
	}
	catch (...)
	{
		Logger::instance().error(
			QString("节点[%1]发生未知错误").arg(node->displayName()));
	}

	return false;
}
