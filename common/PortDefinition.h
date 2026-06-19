// core/PortDefinition.h
#pragma once

#include "DataType.h"
#include <QString>
#include <QSet>

// 端口定义，描述一个端口的元信息
struct PortDefinition
{
	QString name;                   // 端口标识名
	DataType dataType;              // 主数据类型
	QSet<DataType> acceptedTypes;   // 输入端口可接受的类型集合
	bool isOutput = false;          // 是否为输出端口
	bool required = true;           // 输入端口是否必需

	// 创建输出端口定义
	static PortDefinition output(const QString& name, DataType type)
	{
		PortDefinition def;
		def.name = name;
		def.dataType = type;
		def.acceptedTypes = { type };
		def.isOutput = true;
		def.required = false;
		return def;
	}

	// 创建单类型输入端口定义
	static PortDefinition input(const QString& name, DataType type, bool required = true)
	{
		PortDefinition def;
		def.name = name;
		def.dataType = type;
		def.acceptedTypes = { type };
		def.isOutput = false;
		def.required = required;
		return def;
	}

	// 创建多类型输入端口定义
	static PortDefinition inputMulti(const QString& name,
									 const QSet<DataType>& accepted,
									 bool required = true)
	{
		PortDefinition def;
		def.name = name;
		def.dataType = *accepted.begin(); // 取第一个作为主类型
		def.acceptedTypes = accepted;
		def.isOutput = false;
		def.required = required;
		return def;
	}

	// 检查某个数据类型是否可被此端口接受
	bool accepts(DataType type) const
	{
		return acceptedTypes.contains(type);
	}
};
