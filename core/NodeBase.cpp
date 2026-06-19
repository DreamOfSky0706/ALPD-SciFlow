// core/NodeBase.cpp
#include "NodeBase.h"
#include "Exceptions.h"
#include "Logger.h"

NodeBase::NodeBase()
{
}

void NodeBase::onParamChanged(const QString& paramName)
{
	Q_UNUSED(paramName);
	invalidateCache();
}

void NodeBase::rebuildPorts()
{
	// 默认无操作
}

bool NodeBase::validate(QString& errorMsg)
{
	// 检查所有必需输入端口是否有数据
	for (const auto& port : m_inputPorts)
	{
		if (!port.required)
		{
			continue;
		}
		auto it = m_inputData.find(port.name);
		if (it == m_inputData.end() || !it.value() || it.value()->isNull())
		{
			errorMsg = QString("输入端口[%1]未接收到数据").arg(port.name);
			return false;
		}
	}
	return true;
}

void NodeBase::onInputConnected(const QString& portName, DataType actualType)
{
	Q_UNUSED(portName);
	Q_UNUSED(actualType);
}

void NodeBase::onInputDisconnected(const QString& portName)
{
	Q_UNUSED(portName);
}

QString NodeBase::id() const
{
	return m_id;
}

void NodeBase::setId(const QString& id)
{
	m_id = id;
}

QString NodeBase::typeName() const
{
	return m_typeName;
}

void NodeBase::setTypeName(const QString& type)
{
	m_typeName = type;
}

QString NodeBase::displayName() const
{
	return m_displayName;
}

void NodeBase::setDisplayName(const QString& name)
{
	m_displayName = name;
}

QPointF NodeBase::position() const
{
	return m_position;
}

void NodeBase::setPosition(const QPointF& pos)
{
	m_position = pos;
}

const QVector<PortDefinition>& NodeBase::inputPorts() const
{
	return m_inputPorts;
}

const QVector<PortDefinition>& NodeBase::outputPorts() const
{
	return m_outputPorts;
}

PortDefinition* NodeBase::findInputPort(const QString& name)
{
	for (auto& port : m_inputPorts)
	{
		if (port.name == name)
		{
			return &port;
		}
	}
	return nullptr;
}

PortDefinition* NodeBase::findOutputPort(const QString& name)
{
	for (auto& port : m_outputPorts)
	{
		if (port.name == name)
		{
			return &port;
		}
	}
	return nullptr;
}

const PortDefinition* NodeBase::findInputPort(const QString& name) const
{
	for (const auto& port : m_inputPorts)
	{
		if (port.name == name)
		{
			return &port;
		}
	}
	return nullptr;
}

const PortDefinition* NodeBase::findOutputPort(const QString& name) const
{
	for (const auto& port : m_outputPorts)
	{
		if (port.name == name)
		{
			return &port;
		}
	}
	return nullptr;
}

const QVector<ParamDefinition>& NodeBase::paramDefinitions() const
{
	return m_paramDefs;
}

QVariant NodeBase::param(const QString& name) const
{
	if (m_params.contains(name))
		return m_params.value(name);
	// 回退到参数定义的默认值
	for (const auto& d : m_paramDefs) {
		if (d.name == name) return d.defaultValue;
	}
	return QVariant();
}

void NodeBase::setParam(const QString& name, const QVariant& value)
{
	m_params[name] = value;
	onParamChanged(name);
}

QVariantMap NodeBase::allParams() const
{
	return m_params;
}

void NodeBase::setAllParams(const QVariantMap& params)
{
	m_params = params;
	invalidateCache();
}

std::shared_ptr<NodeData> NodeBase::getInput(const QString& portName) const
{
	auto it = m_inputData.find(portName);
	if (it != m_inputData.end())
	{
		return it.value();
	}
	return nullptr;
}

void NodeBase::setOutput(const QString& portName, std::shared_ptr<NodeData> data)
{
	m_outputData[portName] = data;
}

void NodeBase::feedInput(const QString& portName, std::shared_ptr<NodeData> data)
{
	m_inputData[portName] = data;
}

std::shared_ptr<NodeData> NodeBase::getOutput(const QString& portName) const
{
	auto it = m_outputData.find(portName);
	if (it != m_outputData.end())
	{
		return it.value();
	}
	return nullptr;
}

bool NodeBase::isCacheValid() const
{
	return m_cacheValid;
}

void NodeBase::invalidateCache()
{
	m_cacheValid = false;
}

void NodeBase::clearOutputs()
{
	m_outputData.clear();
	m_cacheValid = false;
}

QString NodeBase::categoryPath() const
{
	return m_categoryPath;
}

void NodeBase::setCategoryPath(const QString& path)
{
	m_categoryPath = path;
}

bool NodeBase::shouldUseFloatingPanel() const
{
	if (m_paramDefs.isEmpty())
	{
		return true;
	}
	// 参数数量不超过4个，且不含复杂控件类型时使用悬浮面板
	int visibleCount = 0;
	for (const auto& def : m_paramDefs)
	{
		if (!def.visible)
		{
			continue;
		}
		visibleCount++;
		// 以下类型属于复杂控件，需要属性面板
		if (def.type == ParamType::CurveEditor
			|| def.type == ParamType::TableEditor
			|| def.type == ParamType::GridEditor
			|| def.type == ParamType::GradientEditor
			|| def.type == ParamType::AnnotationList
			|| def.type == ParamType::FlowchartSteps
			|| def.type == ParamType::KernelMatrix
			|| def.type == ParamType::PointList
			|| def.type == ParamType::LegendList
			|| def.type == ParamType::LineStyleList)
		{
			return false;
		}
	}
	return visibleCount <= 4;
}

void NodeBase::addInputPort(const QString& name, DataType type, bool required)
{
	m_inputPorts.append(PortDefinition::input(name, type, required));
}

void NodeBase::addInputPort(const QString& name,
							const QSet<DataType>& acceptedTypes,
							bool required)
{
	m_inputPorts.append(PortDefinition::inputMulti(name, acceptedTypes, required));
}

void NodeBase::addOutputPort(const QString& name, DataType type)
{
	m_outputPorts.append(PortDefinition::output(name, type));
}

void NodeBase::addParam(const QString& name,
						const QString& label,
						ParamType type,
						const QVariant& defaultValue,
						const QVariantMap& constraints)
{
	ParamDefinition def;
	def.name = name;
	def.label = label;
	def.type = type;
	def.defaultValue = defaultValue;
	def.constraints = constraints;
	def.visible = true;

	// 提取条件显示表达式
	if (constraints.contains("visible_when"))
	{
		def.visibleCondition = constraints.value("visible_when").toString();
	}

	m_paramDefs.append(def);

	// 设置默认参数值
	if (!m_params.contains(name))
	{
		m_params[name] = defaultValue;
	}
}

void NodeBase::reportError(const QString& msg)
{
	Logger::instance().error(QString("节点[%1]：%2").arg(m_displayName, msg));
	throw NodeProcessException(m_id, msg);
}

void NodeBase::reportWarning(const QString& msg)
{
	Logger::instance().warning(QString("节点[%1]：%2").arg(m_displayName, msg));
}

void NodeBase::clearInputPorts()
{
	m_inputPorts.clear();
}

void NodeBase::clearOutputPorts()
{
	m_outputPorts.clear();
}
