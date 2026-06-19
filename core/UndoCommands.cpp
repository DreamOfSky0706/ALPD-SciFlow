// core/UndoCommands.cpp
#include "UndoCommands.h"
#include "WorkflowGraph.h"
#include "NodeBase.h"

void ChangeParamCommand::undo()
{
	NodeBase* node = m_graph->nodeById(m_nodeId);
	if (node) {
		node->setParam(m_paramName, m_oldValue);
		m_graph->invalidateCache(m_nodeId);
	}
}

void ChangeParamCommand::redo()
{
	NodeBase* node = m_graph->nodeById(m_nodeId);
	if (node) {
		node->setParam(m_paramName, m_newValue);
		m_graph->invalidateCache(m_nodeId);
	}
}

bool ChangeParamCommand::mergeWith(const QUndoCommand* other)
{
	auto* cmd = dynamic_cast<const ChangeParamCommand*>(other);
	if (!cmd || cmd->m_nodeId != m_nodeId || cmd->m_paramName != m_paramName)
		return false;
	m_newValue = cmd->m_newValue;
	return true;
}
