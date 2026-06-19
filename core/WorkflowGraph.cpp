// core/WorkflowGraph.cpp
#include "WorkflowGraph.h"
#include "NodeFactory.h"
#include "ExecutionEngine.h"
#include "UndoCommands.h"
#include "Logger.h"
#include "Exceptions.h"
#include <QUuid>
#include <QQueue>
#include <QSet>

WorkflowGraph::WorkflowGraph(QObject* parent)
	: QObject(parent)
	, m_engine(std::make_unique<ExecutionEngine>(this))
{
	m_undoStack.setUndoLimit(100);
}

WorkflowGraph::~WorkflowGraph() = default;

NodeBase* WorkflowGraph::addNode(const QString& typeName, const QPointF& position,
                                  const QString& nodeId)
{
	auto node = NodeFactory::instance().createNode(typeName);
	if (!node)
	{
		Logger::instance().error(QString("无法创建节点：未知类型[%1]").arg(typeName));
		return nullptr;
	}

	QString id = nodeId.isEmpty() ? generateNodeId() : nodeId;
	node->setId(id);
	node->setPosition(position);

	NodeBase* rawPtr = node.get();
	m_nodes[id] = std::move(node);

	Logger::instance().info(QString("添加节点: %1").arg(rawPtr->displayName()));
	emit nodeAdded(id);
	return rawPtr;
}

void WorkflowGraph::removeNode(const QString& nodeId)
{
	if (m_nodes.count(nodeId) == 0)
	{
		return;
	}

	// 记录节点名称（erase后会销毁）
	QString name = m_nodes[nodeId]->displayName();

	// 先删除所有关联连线
	removeConnectionsOfNode(nodeId);

	m_nodes.erase(nodeId);
	Logger::instance().info(QString("删除节点: %1").arg(name));
	emit nodeRemoved(nodeId);
}

NodeBase* WorkflowGraph::nodeById(const QString& id) const
{
	auto it = m_nodes.find(id);
	if (it != m_nodes.end())
	{
		return it->second.get();
	}
	return nullptr;
}

QList<NodeBase*> WorkflowGraph::allNodes() const
{
	QList<NodeBase*> result;
	for (const auto& pair : m_nodes)
	{
		result.append(pair.second.get());
	}
	return result;
}

QStringList WorkflowGraph::allNodeIds() const
{
	QStringList result;
	for (const auto& pair : m_nodes)
	{
		result.append(pair.first);
	}
	return result;
}

bool WorkflowGraph::addConnection(const QString& srcNodeId,
								  const QString& srcPort,
								  const QString& dstNodeId,
								  const QString& dstPort)
{
	// 检查节点存在
	NodeBase* srcNode = nodeById(srcNodeId);
	NodeBase* dstNode = nodeById(dstNodeId);
	if (!srcNode || !dstNode)
	{
		Logger::instance().error("连线失败：节点不存在");
		return false;
	}

	// 检查端口存在
	const PortDefinition* outPort = srcNode->findOutputPort(srcPort);
	const PortDefinition* inPort = dstNode->findInputPort(dstPort);
	if (!outPort || !inPort)
	{
		Logger::instance().error("连线失败：端口不存在");
		return false;
	}

	// 检查类型兼容性
	if (!inPort->accepts(outPort->dataType))
	{
		Logger::instance().error(
			QString("无法连接：输出类型为%1，但目标端口只接受%2")
			.arg(dataTypeName(outPort->dataType),
				 dataTypeName(inPort->dataType)));
		return false;
	}

	// 检查是否已存在相同连线
	Connection newConn(srcNodeId, srcPort, dstNodeId, dstPort);
	for (const auto& conn : m_connections)
	{
		if (conn == newConn)
		{
			return true; // 已存在，静默成功
		}
	}

	// 输入端口只能有一条连线，断开已有连线
	Connection* existing = findConnectionToInput(dstNodeId, dstPort);
	if (existing)
	{
		removeConnection(existing->sourceNodeId(),
						 existing->sourcePortName(),
						 existing->targetNodeId(),
						 existing->targetPortName());
	}

	// 禁止自环
	if (srcNodeId == dstNodeId)
	{
		Logger::instance().error("不允许建立此连线：不能连接自身");
		return false;
	}

	// 检查环路（ListIterator的feedback端口仅允许来自其下游节点的回连）
	if (dstPort == "input_iteration_result")
	{
		// 仅当src在dst的下游时允许（迭代子图末端回连）
		QSet<QString> srcDescendants = findDescendants(srcNodeId);
		if (!srcDescendants.contains(dstNodeId))
		{
			Logger::instance().error("迭代回连只能从下游节点连入");
			return false;
		}
	}
	else
	{
		QSet<QString> descendants = findDescendants(dstNodeId);
		if (descendants.contains(srcNodeId))
		{
			Logger::instance().error("不允许建立此连线：将形成循环依赖");
			return false;
		}
	}

	m_connections.append(newConn);

	// 通知目标节点有新连入
	dstNode->onInputConnected(dstPort, outPort->dataType);

	// 失效下游缓存
	invalidateDownstream(dstNodeId);

	emit connectionAdded(newConn);
	return true;
}

void WorkflowGraph::removeConnection(const QString& srcNodeId,
									 const QString& srcPort,
									 const QString& dstNodeId,
									 const QString& dstPort)
{
	Connection target(srcNodeId, srcPort, dstNodeId, dstPort);
	for (int i = 0; i < m_connections.size(); ++i)
	{
		if (m_connections[i] == target)
		{
			m_connections.removeAt(i);

			// 通知目标节点连线断开
			NodeBase* dstNode = nodeById(dstNodeId);
			if (dstNode)
			{
				dstNode->onInputDisconnected(dstPort);
			}

			invalidateDownstream(dstNodeId);
			Logger::instance().info(QString("断开连线: %1.%2 → %3.%4")
				.arg(srcNodeId).arg(srcPort).arg(dstNodeId).arg(dstPort));
			emit connectionRemoved(target);
			return;
		}
	}
}

void WorkflowGraph::removeConnectionsOfNode(const QString& nodeId)
{
	QVector<Connection> toRemove;
	for (const auto& conn : m_connections)
	{
		if (conn.involvesNode(nodeId))
		{
			toRemove.append(conn);
		}
	}
	for (const auto& conn : toRemove)
	{
		removeConnection(conn.sourceNodeId(),
						 conn.sourcePortName(),
						 conn.targetNodeId(),
						 conn.targetPortName());
	}
}

Connection* WorkflowGraph::findConnectionToInput(const QString& nodeId,
												 const QString& portName)
{
	for (auto& conn : m_connections)
	{
		if (conn.targetNodeId() == nodeId && conn.targetPortName() == portName)
		{
			return &conn;
		}
	}
	return nullptr;
}

QVector<Connection> WorkflowGraph::findConnectionsFromOutput(
	const QString& nodeId,
	const QString& portName) const
{
	QVector<Connection> result;
	for (const auto& conn : m_connections)
	{
		if (conn.sourceNodeId() == nodeId && conn.sourcePortName() == portName)
		{
			result.append(conn);
		}
	}
	return result;
}

const QVector<Connection>& WorkflowGraph::allConnections() const
{
	return m_connections;
}

bool WorkflowGraph::hasCycle() const
{
	// DFS染色法检测环路
	QMap<QString, int> color; // 0=白 1=灰 2=黑
	for (const auto& pair : m_nodes)
	{
		color[pair.first] = 0;
	}

	std::function<bool(const QString&)> dfs = [&](const QString& nodeId) -> bool
		{
			color[nodeId] = 1;
			// 找此节点的所有直接后继
			for (const auto& conn : m_connections)
			{
				if (conn.sourceNodeId() != nodeId)
				{
					continue;
				}
				const QString& next = conn.targetNodeId();
				if (color[next] == 1)
				{
					return true; // 发现环
				}
				if (color[next] == 0 && dfs(next))
				{
					return true;
				}
			}
			color[nodeId] = 2;
			return false;
		};

	for (const auto& pair : m_nodes)
	{
		if (color[pair.first] == 0 && dfs(pair.first))
		{
			return true;
		}
	}
	return false;
}

QVector<NodeBase*> WorkflowGraph::topologicalSort() const
{
	// Kahn算法
	QMap<QString, int> inDegree;
	for (const auto& pair : m_nodes)
	{
		inDegree[pair.first] = 0;
	}
	for (const auto& conn : m_connections)
	{
		inDegree[conn.targetNodeId()]++;
	}

	QQueue<QString> queue;
	for (auto it = inDegree.begin(); it != inDegree.end(); ++it)
	{
		if (it.value() == 0)
		{
			queue.enqueue(it.key());
		}
	}

	QVector<NodeBase*> sorted;
	while (!queue.isEmpty())
	{
		QString current = queue.dequeue();
		NodeBase* node = nodeById(current);
		if (node)
		{
			sorted.append(node);
		}

		for (const auto& conn : m_connections)
		{
			if (conn.sourceNodeId() != current)
			{
				continue;
			}
			inDegree[conn.targetNodeId()]--;
			if (inDegree[conn.targetNodeId()] == 0)
			{
				queue.enqueue(conn.targetNodeId());
			}
		}
	}

	return sorted;
}

QVector<NodeBase*> WorkflowGraph::topologicalSortPartial(
	const QString& targetNodeId,
	PartialDirection direction) const
{
	QSet<QString> relevantIds;

	if (direction == Upstream)
	{
		// 包含目标节点及其所有祖先
		relevantIds = findAncestors(targetNodeId);
		relevantIds.insert(targetNodeId);
	}
	else
	{
		// 包含目标节点及其所有后代
		relevantIds = findDescendants(targetNodeId);
		relevantIds.insert(targetNodeId);
	}

	// 对子集做拓扑排序
	QMap<QString, int> inDegree;
	for (const auto& id : relevantIds)
	{
		inDegree[id] = 0;
	}
	for (const auto& conn : m_connections)
	{
		if (relevantIds.contains(conn.sourceNodeId())
			&& relevantIds.contains(conn.targetNodeId()))
		{
			inDegree[conn.targetNodeId()]++;
		}
	}

	QQueue<QString> queue;
	for (auto it = inDegree.begin(); it != inDegree.end(); ++it)
	{
		if (it.value() == 0)
		{
			queue.enqueue(it.key());
		}
	}

	QVector<NodeBase*> sorted;
	while (!queue.isEmpty())
	{
		QString current = queue.dequeue();
		NodeBase* node = nodeById(current);
		if (node)
		{
			sorted.append(node);
		}

		for (const auto& conn : m_connections)
		{
			if (conn.sourceNodeId() != current)
			{
				continue;
			}
			if (!relevantIds.contains(conn.targetNodeId()))
			{
				continue;
			}
			inDegree[conn.targetNodeId()]--;
			if (inDegree[conn.targetNodeId()] == 0)
			{
				queue.enqueue(conn.targetNodeId());
			}
		}
	}

	return sorted;
}

void WorkflowGraph::executeAll()
{
	m_engine->executeAll();
}

int WorkflowGraph::lastFailCount() const
{
	return m_engine->lastFailCount();
}

void WorkflowGraph::executePartial(const QString& targetNodeId,
								   PartialDirection direction)
{
	m_engine->executePartial(targetNodeId, direction);
}

void WorkflowGraph::changeParam(const QString& nodeId,
								const QString& paramName,
								const QVariant& newValue,
								bool pushUndo)
{
	NodeBase* node = nodeById(nodeId);
	if (!node) return;

	QVariant oldVal = node->param(paramName);
	// 值未变化则跳过
	if (oldVal == newValue) return;

	node->setParam(paramName, newValue);
	invalidateDownstream(nodeId);
	if (pushUndo)
		m_undoStack.push(new ChangeParamCommand(this, nodeId, paramName, oldVal, newValue));
	emit paramChanged(nodeId, paramName);
	// 通知Scene可能的端口变化（动态端口节点需要更新图形项）
	emit nodePortsChanged(nodeId);
}

void WorkflowGraph::clear()
{
	// 逐个删除节点和连线（触发信号让Scene同步清理图形项）
	while (!m_connections.isEmpty()) {
		auto conn = m_connections.first();
		removeConnection(conn.sourceNodeId(), conn.sourcePortName(),
		                 conn.targetNodeId(), conn.targetPortName());
	}
	QStringList ids = allNodeIds();
	for (const auto& id : ids) {
		removeNode(id);
	}
	m_orderHints.clear();
	m_undoStack.clear();
}

QUndoStack* WorkflowGraph::undoStack()
{
	return &m_undoStack;
}

void WorkflowGraph::setExecutionOrderHint(const QString& contextNodeId,
										  const QString& contextPort,
										  const QStringList& orderedSources)
{
	QString key = contextNodeId + ":" + contextPort;
	m_orderHints[key] = orderedSources;
}

QStringList WorkflowGraph::executionOrderHint(const QString& contextNodeId,
											  const QString& contextPort) const
{
	QString key = contextNodeId + ":" + contextPort;
	return m_orderHints.value(key);
}

QVariantList WorkflowGraph::allExecutionOrderHints() const
{
	QVariantList result;
	for (auto it = m_orderHints.begin(); it != m_orderHints.end(); ++it)
	{
		QStringList parts = it.key().split(":");
		if (parts.size() != 2)
		{
			continue;
		}
		QVariantMap hint;
		hint["context_node"] = parts[0];
		hint["context_port"] = parts[1];
		hint["ordered_sources"] = QVariant(it.value());
		result.append(hint);
	}
	return result;
}

void WorkflowGraph::setAllExecutionOrderHints(const QVariantList& hints)
{
	m_orderHints.clear();
	for (const auto& item : hints)
	{
		QVariantMap hint = item.toMap();
		QString nodeId = hint.value("context_node").toString();
		QString port = hint.value("context_port").toString();
		QStringList ordered = hint.value("ordered_sources").toStringList();
		if (!nodeId.isEmpty() && !port.isEmpty())
		{
			m_orderHints[nodeId + ":" + port] = ordered;
		}
	}
}

QString WorkflowGraph::generateNodeId() const
{
	QString id;
	do
	{
		id = "n" + QUuid::createUuid().toString(QUuid::Id128).left(8);
	}
	while (m_nodes.count(id) > 0);
	return id;
}

QSet<QString> WorkflowGraph::findAncestors(const QString& nodeId) const
{
	QSet<QString> ancestors;
	QQueue<QString> queue;

	// 找到所有连向nodeId的源节点
	for (const auto& conn : m_connections)
	{
		if (conn.targetNodeId() == nodeId)
		{
			queue.enqueue(conn.sourceNodeId());
		}
	}

	while (!queue.isEmpty())
	{
		QString current = queue.dequeue();
		if (ancestors.contains(current))
		{
			continue;
		}
		ancestors.insert(current);
		for (const auto& conn : m_connections)
		{
			if (conn.targetNodeId() == current)
			{
				queue.enqueue(conn.sourceNodeId());
			}
		}
	}

	return ancestors;
}

QSet<QString> WorkflowGraph::findDescendants(const QString& nodeId) const
{
	QSet<QString> descendants;
	QQueue<QString> queue;

	for (const auto& conn : m_connections)
	{
		if (conn.sourceNodeId() == nodeId)
		{
			queue.enqueue(conn.targetNodeId());
		}
	}

	while (!queue.isEmpty())
	{
		QString current = queue.dequeue();
		if (descendants.contains(current))
		{
			continue;
		}
		descendants.insert(current);
		for (const auto& conn : m_connections)
		{
			if (conn.sourceNodeId() == current)
			{
				queue.enqueue(conn.targetNodeId());
			}
		}
	}

	return descendants;
}

void WorkflowGraph::invalidateCache(const QString& nodeId)
{
	invalidateDownstream(nodeId);
}

void WorkflowGraph::invalidateDownstream(const QString& nodeId)
{
	NodeBase* node = nodeById(nodeId);
	if (node)
	{
		node->invalidateCache();
	}
	QSet<QString> descendants = findDescendants(nodeId);
	for (const auto& id : descendants)
	{
		NodeBase* desc = nodeById(id);
		if (desc)
		{
			desc->invalidateCache();
		}
	}
}
