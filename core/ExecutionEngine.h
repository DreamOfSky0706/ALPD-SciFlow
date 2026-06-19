// core/ExecutionEngine.h
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class WorkflowGraph;
class NodeBase;

// 工作流执行引擎，负责按拓扑顺序执行节点
class ExecutionEngine : public QObject
{
	Q_OBJECT

public:
	explicit ExecutionEngine(WorkflowGraph* graph, QObject* parent = nullptr);

	// 执行全部节点
	void executeAll();

	// 部分执行
	void executePartial(const QString& targetNodeId, int direction);
	// 获取上次执行的失败节点数
	int lastFailCount() const { return m_lastFailCount; }

private:
	// 执行一组已排好序的节点
	void executeNodeList(const QVector<NodeBase*>& nodes);

	// 在执行某个节点前，将上游数据注入到它的输入端口
	void feedInputsForNode(NodeBase* node);

	// 执行单个节点，返回true表示成功
	bool executeSingleNode(NodeBase* node);

	WorkflowGraph* m_graph;
	int m_lastFailCount = 0;
};
