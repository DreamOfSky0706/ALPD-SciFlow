// core/NodeBase.h
#pragma once

#include "DataType.h"
#include "PortDefinition.h"
#include "NodeData.h"
#include "Utility.h"
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QColor>
#include <QPointF>
#include <QMap>
#include <memory>
#include <opencv2/core.hpp>

// 参数类型枚举，决定属性面板中使用的控件类型
enum class ParamType
{
	Int,            // QSpinBox
	Double,         // QDoubleSpinBox
	IntSlider,      // QSlider + QSpinBox
	DoubleSlider,   // QSlider + QDoubleSpinBox
	String,         // QLineEdit 单行
	MultiLineString,// QTextEdit 多行
	FilePath,       // QLineEdit + 浏览按钮（打开文件）
	SaveFilePath,   // QLineEdit + 浏览按钮（保存文件）
	DirPath,        // QLineEdit + 浏览按钮（选择目录）
	Combo,          // QComboBox
	Bool,           // QCheckBox
	Color,          // 颜色选取器
	Font,           // QFontComboBox
	CurveEditor,    // 曲线编辑器控件
	TableEditor,    // 表格编辑器控件
	GridEditor,     // 网格编辑器控件（多图排版专用）
	GradientEditor, // 渐变色停靠点编辑器
	AnnotationList, // 标注列表编辑器
	FlowchartSteps, // 流程图步骤编辑器
	KernelMatrix,   // 卷积核矩阵编辑器
	PointList,      // 坐标点列表编辑器
	MultiCombo,     // 多选下拉框
	LegendList,     // 图例项列表编辑器
	LineStyleList   // 折线样式列表编辑器
};

// 单个参数的定义
struct ParamDefinition
{
	QString name;               // 参数标识键
	QString label;              // 显示标签（中文）
	ParamType type;             // 控件类型
	QVariant defaultValue;      // 默认值
	QVariantMap constraints;    // 约束信息，如min/max/step/options等
	bool visible = true;        // 当前是否可见（条件显示用）
	QString visibleCondition;   // 控制可见性的条件表达式（如"mode==指定尺寸"）
};

// 所有节点的抽象基类
class NodeBase
{
public:
	NodeBase();
	virtual ~NodeBase() = default;

	// 子类必须实现：声明端口和参数
	virtual void defineNode() = 0;

	// 子类必须实现：执行处理逻辑
	virtual void process() = 0;

	// 参数修改回调，子类可重写实现联动
	virtual void onParamChanged(const QString& paramName);

	// 动态端口重建，子类可重写
	virtual void rebuildPorts();

	// 执行前校验，返回false时errorMsg中含错误描述
	virtual bool validate(QString& errorMsg);

	// 输入连线变化回调
	virtual void onInputConnected(const QString& portName, DataType actualType);
	virtual void onInputDisconnected(const QString& portName);

	// 迭代器节点接口（ListIterator等循环容器）
	virtual bool isIterator() const { return false; }
	virtual void advanceIteration() {}
	virtual bool hasMore() const { return false; }
	virtual int  currentIndex() const { return 0; }

	// 基本属性的读写
	QString id() const;
	void setId(const QString& id);

	QString typeName() const;
	void setTypeName(const QString& type);

	QString displayName() const;
	void setDisplayName(const QString& name);

	QPointF position() const;
	void setPosition(const QPointF& pos);

	// 端口访问
	const QVector<PortDefinition>& inputPorts() const;
	const QVector<PortDefinition>& outputPorts() const;

	PortDefinition* findInputPort(const QString& name);
	PortDefinition* findOutputPort(const QString& name);
	const PortDefinition* findInputPort(const QString& name) const;
	const PortDefinition* findOutputPort(const QString& name) const;

	// 参数访问
	const QVector<ParamDefinition>& paramDefinitions() const;
	QVariant param(const QString& name) const;
	void setParam(const QString& name, const QVariant& value);
	QVariantMap allParams() const;
	void setAllParams(const QVariantMap& params);

	// 端口数据读写（供process()内部使用）
	std::shared_ptr<NodeData> getInput(const QString& portName) const;
	void setOutput(const QString& portName, std::shared_ptr<NodeData> data);

	// 外部（执行引擎）向输入端口注入数据
	void feedInput(const QString& portName, std::shared_ptr<NodeData> data);

	// 获取输出端口的缓存数据
	std::shared_ptr<NodeData> getOutput(const QString& portName) const;

	// 缓存管理
	bool isCacheValid() const;
	void invalidateCache();
	void clearOutputs();

	// 节点所属分类路径（如"滤波与模糊/模糊"），由工厂设置
	QString categoryPath() const;
	void setCategoryPath(const QString& path);

	// 判断参数面板是否应该使用悬浮模式
	bool shouldUseFloatingPanel() const;

protected:
	// 子类在defineNode()中调用这些方法声明端口和参数
	void addInputPort(const QString& name, DataType type, bool required = true);
	void addInputPort(const QString& name,
					  const QSet<DataType>& acceptedTypes,
					  bool required = true);
	void addOutputPort(const QString& name, DataType type);

	void addParam(const QString& name,
				  const QString& label,
				  ParamType type,
				  const QVariant& defaultValue,
				  const QVariantMap& constraints = {});

	// 报告错误，抛出NodeProcessException
	void reportError(const QString& msg);

	// 报告警告，记录到日志但不中断执行
	void reportWarning(const QString& msg);

	// 清除所有端口定义（rebuildPorts时用）
	void clearInputPorts();
	void clearOutputPorts();

private:
	QString m_id;
	QString m_typeName;
	QString m_displayName;
	QString m_categoryPath;
	QPointF m_position;

	QVector<PortDefinition> m_inputPorts;
	QVector<PortDefinition> m_outputPorts;
	QVector<ParamDefinition> m_paramDefs;
	QVariantMap m_params;

	// 输入端口接收到的数据（由执行引擎在process()前注入）
	QMap<QString, std::shared_ptr<NodeData>> m_inputData;

	// 输出端口产生的数据（由process()内部写入）
	QMap<QString, std::shared_ptr<NodeData>> m_outputData;

	bool m_cacheValid = false;
};
