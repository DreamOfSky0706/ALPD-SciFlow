// core/Connection.h
#pragma once

#include <QString>

// 描述两个端口之间的一条连线
class Connection
{
public:
	Connection() = default;
	Connection(const QString& srcNodeId,
			   const QString& srcPortName,
			   const QString& dstNodeId,
			   const QString& dstPortName);

	QString sourceNodeId() const;
	QString sourcePortName() const;
	QString targetNodeId() const;
	QString targetPortName() const;

	// 判断此连线是否涉及指定节点
	bool involvesNode(const QString& nodeId) const;

	bool operator==(const Connection& other) const;
	bool operator!=(const Connection& other) const;

private:
	QString m_srcNodeId;
	QString m_srcPortName;
	QString m_dstNodeId;
	QString m_dstPortName;
};
