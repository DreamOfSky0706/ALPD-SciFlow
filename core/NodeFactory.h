#pragma once
// core/NodeFactory.h
#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <functional>
#include <memory>

class NodeBase;

// 节点注册信息
struct NodeRegistration
{
	QString typeName;       // 注册键，如"GaussianBlur"
	QString categoryPath;  // 分类路径，如"滤波与模糊/模糊"
	QString displayName;   // 中文显示名，如"高斯模糊"
	std::function<std::unique_ptr<NodeBase>()> creator;
};

// 节点工厂，单例模式，管理所有节点类型的注册和创建
class NodeFactory
{
public:
	static NodeFactory& instance();

	// 注册节点类型
	template<typename T>
	void registerNode(const QString& typeName,
					  const QString& categoryPath,
					  const QString& displayName)
	{
		NodeRegistration reg;
		reg.typeName = typeName;
		reg.categoryPath = categoryPath;
		reg.displayName = displayName;
		reg.creator = []() -> std::unique_ptr<NodeBase>
			{
				return std::make_unique<T>();
			};
		m_registrations[typeName] = reg;
	}

	// 根据类型名创建节点实例，返回空指针表示类型不存在
	std::unique_ptr<NodeBase> createNode(const QString& typeName) const;

	// 获取所有已注册的类型名
	QStringList allTypeNames() const;

	// 获取指定类型的注册信息，不存在时返回nullptr
	const NodeRegistration* registration(const QString& typeName) const;

	// 获取所有注册信息，用于构建工具箱树
	QList<NodeRegistration> allRegistrations() const;

	// 检查某个类型是否已注册
	bool hasType(const QString& typeName) const;

private:
	NodeFactory() = default;
	~NodeFactory() = default;
	NodeFactory(const NodeFactory&) = delete;
	NodeFactory& operator=(const NodeFactory&) = delete;

	QMap<QString, NodeRegistration> m_registrations;
};
