// core/UndoCommands.h
#pragma once
#include <QUndoCommand>
#include <QPointF>
#include <QVariant>

class WorkflowGraph;

// 参数修改命令（支持mergeWith合并连续修改）
class ChangeParamCommand : public QUndoCommand
{
public:
	ChangeParamCommand(WorkflowGraph* graph, const QString& nodeId,
	                   const QString& paramName, const QVariant& oldVal,
	                   const QVariant& newVal)
		: m_graph(graph), m_nodeId(nodeId), m_paramName(paramName)
		, m_oldValue(oldVal), m_newValue(newVal)
	{
		setText(QString("修改参数 %1").arg(paramName));
	}
	void undo() override;
	void redo() override;
	int id() const override { return 1; }
	bool mergeWith(const QUndoCommand* other) override;

private:
	WorkflowGraph* m_graph;
	QString m_nodeId, m_paramName;
	QVariant m_oldValue, m_newValue;
};
