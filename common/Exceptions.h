// common/Exceptions.h
#pragma once

#include <stdexcept>
#include <QString>

// 节点处理过程中的异常，携带节点ID和错误描述
class NodeProcessException : public std::runtime_error
{
public:
	NodeProcessException(const QString& nodeId, const QString& message)
		: std::runtime_error(message.toStdString())
		, m_nodeId(nodeId)
		, m_message(message)
	{
	}

	QString nodeId() const
	{
		return m_nodeId;
	}

	QString message() const
	{
		return m_message;
	}

private:
	QString m_nodeId;
	QString m_message;
};

// 工作流结构异常，如环路检测失败
class WorkflowException : public std::runtime_error
{
public:
	explicit WorkflowException(const QString& message)
		: std::runtime_error(message.toStdString())
		, m_message(message)
	{
	}

	QString message() const
	{
		return m_message;
	}

private:
	QString m_message;
};

// 序列化与反序列化异常
class SerializationException : public std::runtime_error
{
public:
	explicit SerializationException(const QString& message)
		: std::runtime_error(message.toStdString())
		, m_message(message)
	{
	}

	QString message() const
	{
		return m_message;
	}

private:
	QString m_message;
};
