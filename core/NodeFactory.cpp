// core/NodeFactory.cpp
#include "NodeFactory.h"
#include "NodeBase.h"

NodeFactory& NodeFactory::instance()
{
	static NodeFactory s_instance;
	return s_instance;
}

std::unique_ptr<NodeBase> NodeFactory::createNode(const QString& typeName) const
{
	auto it = m_registrations.find(typeName);
	if (it == m_registrations.end())
	{
		return nullptr;
	}

	auto node = it->creator();
	if (node)
	{
		node->setTypeName(typeName);
		node->setDisplayName(it->displayName);
		node->setCategoryPath(it->categoryPath);
		node->defineNode();
	}
	return node;
}

QStringList NodeFactory::allTypeNames() const
{
	return m_registrations.keys();
}

const NodeRegistration* NodeFactory::registration(const QString& typeName) const
{
	auto it = m_registrations.find(typeName);
	if (it == m_registrations.end())
	{
		return nullptr;
	}
	return &(*it);
}

QList<NodeRegistration> NodeFactory::allRegistrations() const
{
	return m_registrations.values();
}

bool NodeFactory::hasType(const QString& typeName) const
{
	return m_registrations.contains(typeName);
}
