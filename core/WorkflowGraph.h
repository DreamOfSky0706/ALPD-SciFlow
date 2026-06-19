// core/WorkflowGraph.h
#pragma once

#include "NodeBase.h"
#include "Connection.h"
#include <QObject>
#include <QMap>
#include <QVector>
#include <QUndoStack>
#include <memory>
#include <map>

class ExecutionEngine;

// 工作流有向无环图，管理节点和连线
class WorkflowGraph : public QObject
{
	Q_OBJECT

public:
	explicit WorkflowGraph(QObject* parent = nullptr);
	~WorkflowGraph();

	// 节点操作
	NodeBase* addNode(const QString& typeName, const QPointF& position = QPointF(),
	                  const QString& nodeId = QString());
	void removeNode(const QString& nodeId);
	NodeBase* nodeById(const QString& id) const;
	QList<NodeBase*> allNodes() const;
	QStringList allNodeIds() const;

	// 连线操作，返回是否成功
	bool addConnection(const QString& srcNodeId,
					   const QString& srcPort,
					   const QString& dstNodeId,
					   const QString& dstPort);
	void removeConnection(const QString& srcNodeId,
						  const QString& srcPort,
						  const QString& dstNodeId,
						  const QString& dstPort);
	void removeConnectionsOfNode(const QString& nodeId);

	// 查找连接到指定输入端口的连线
	Connection* findConnectionToInput(const QString& nodeId,
									  const QString& portName);

	// 查找从指定输出端口出发的所有连线
	QVector<Connection> findConnectionsFromOutput(const QString& nodeId,
												  const QString& portName) const;

	// 获取所有连线
	const QVector<Connection>& allConnections() const;

	// DAG环路检测，返回true表示存在环
	bool hasCycle() const;

	// 拓扑排序，返回按执行顺序排列的节点列表
	QVector<NodeBase*> topologicalSort() const;

	// 部分拓扑排序
	enum PartialDirection
	{
		Upstream,   // 从数据源到目标节点
		Downstream  // 从目标节点到末端
	};
	QVector<NodeBase*> topologicalSortPartial(const QString& targetNodeId,
											  PartialDirection direction) const;

	// 执行全部
	void executeAll();
	int lastFailCount() const;

	// 部分执行
	void executePartial(const QString& targetNodeId, PartialDirection direction);

	// 参数修改（推入撤销栈）
	void changeParam(const QString& nodeId,
					 const QString& paramName,
					 const QVariant& newValue,
					 bool pushUndo = true);
	void invalidateCache(const QString& nodeId);

	// 清空全部节点和连线
	void clear();

	// 撤销栈
	QUndoStack* undoStack();

	// 工作流排序提示（无依赖节点排序记忆）
	void setExecutionOrderHint(const QString& contextNodeId,
							   const QString& contextPort,
							   const QStringList& orderedSources);
	QStringList executionOrderHint(const QString& contextNodeId,
								   const QString& contextPort) const;
	QVariantList allExecutionOrderHints() const;
	void setAllExecutionOrderHints(const QVariantList& hints);

	// 生成唯一节点ID
	QString generateNodeId() const;

signals:
	void nodeAdded(const QString& nodeId);
	void nodeRemoved(const QString& nodeId);
	void connectionAdded(const Connection& conn);
	void connectionRemoved(const Connection& conn);
	void paramChanged(const QString& nodeId, const QString& paramName);
	void nodePortsChanged(const QString& nodeId);
	void executionStarted();
	void executionProgress(int current, int total, const QString& nodeName);
	void executionFinished(int successCount, int failCount, int skipCount);

private:
	// 查找指定节点的所有上游祖先节点ID
	QSet<QString> findAncestors(const QString& nodeId) const;

	// 查找指定节点的所有下游后代节点ID
	QSet<QString> findDescendants(const QString& nodeId) const;

	// 失效指定节点及其所有下游节点的缓存
	void invalidateDownstream(const QString& nodeId);

	std::map<QString, std::unique_ptr<NodeBase>> m_nodes;
	QVector<Connection> m_connections;
	QUndoStack m_undoStack;

	// 执行排序提示，key为"nodeId:portName"
	QMap<QString, QStringList> m_orderHints;

	std::unique_ptr<ExecutionEngine> m_engine;
};
