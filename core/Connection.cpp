// core/Connection.cpp
#include "Connection.h"

Connection::Connection(const QString& srcNodeId,
					   const QString& srcPortName,
					   const QString& dstNodeId,
					   const QString& dstPortName)
	: m_srcNodeId(srcNodeId)
	, m_srcPortName(srcPortName)
	, m_dstNodeId(dstNodeId)
	, m_dstPortName(dstPortName)
{
}

QString Connection::sourceNodeId() const
{
	return m_srcNodeId;
}

QString Connection::sourcePortName() const
{
	return m_srcPortName;
}

QString Connection::targetNodeId() const
{
	return m_dstNodeId;
}

QString Connection::targetPortName() const
{
	return m_dstPortName;
}

bool Connection::involvesNode(const QString& nodeId) const
{
	return m_srcNodeId == nodeId || m_dstNodeId == nodeId;
}

bool Connection::operator==(const Connection& other) const
{
	return m_srcNodeId == other.m_srcNodeId
		&& m_srcPortName == other.m_srcPortName
		&& m_dstNodeId == other.m_dstNodeId
		&& m_dstPortName == other.m_dstPortName;
}

bool Connection::operator!=(const Connection& other) const
{
	return !(*this == other);
}
