// io/WorkflowSerializer.cpp
#include "WorkflowSerializer.h"
#include "WorkflowGraph.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "Logger.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

double WorkflowSerializer::s_zoom = 1.0;
double WorkflowSerializer::s_cx = 400;
double WorkflowSerializer::s_cy = 300;
void WorkflowSerializer::setCanvasView(double z, double x, double y) { s_zoom=z; s_cx=x; s_cy=y; }
void WorkflowSerializer::getCanvasView(double& z, double& x, double& y) { z=s_zoom; x=s_cx; y=s_cy; }

bool WorkflowSerializer::save(WorkflowGraph* graph, const QString& filePath)
{
	QJsonObject root;
	root["format_version"] = "1.0";
	root["workflow_name"] = QFileInfo(filePath).baseName();
	root["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
	root["modified_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

	QJsonObject canvasView;
	canvasView["zoom"] = s_zoom;
	canvasView["center_x"] = s_cx;
	canvasView["center_y"] = s_cy;
	root["canvas_view"] = canvasView;

	QJsonArray nodesArr;
	for (const auto* node : graph->allNodes())
	{
		QJsonObject nObj;
		nObj["id"] = node->id();
		nObj["type"] = node->typeName();
		nObj["display_name"] = node->displayName();

		QJsonObject pos;
		pos["x"] = node->position().x();
		pos["y"] = node->position().y();
		nObj["position"] = pos;

		QJsonObject params = QJsonObject::fromVariantMap(node->allParams());
		nObj["params"] = params;

		nodesArr.append(nObj);
	}
	root["nodes"] = nodesArr;

	QJsonArray connsArr;
	for (const auto& conn : graph->allConnections())
	{
		QJsonObject cObj;
		cObj["from_node"] = conn.sourceNodeId();
		cObj["from_port"] = conn.sourcePortName();
		cObj["to_node"] = conn.targetNodeId();
		cObj["to_port"] = conn.targetPortName();
		connsArr.append(cObj);
	}
	root["connections"] = connsArr;

	QJsonArray hintsArr = QJsonArray::fromVariantList(graph->allExecutionOrderHints());
	root["execution_order_hints"] = hintsArr;

	QJsonDocument doc(root);
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly))
	{
		Logger::instance().error(QString("无法写入文件：%1").arg(filePath));
		return false;
	}
	file.write(doc.toJson(QJsonDocument::Indented));
	file.close();
	return true;
}

bool WorkflowSerializer::load(const QString& filePath, WorkflowGraph* graph)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		Logger::instance().error(QString("无法打开文件：%1").arg(filePath));
		return false;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(data, &err);
	if (err.error != QJsonParseError::NoError)
	{
		Logger::instance().error(QString("JSON解析错误：%1").arg(err.errorString()));
		return false;
	}

	QJsonObject root = doc.object();
	QString version = root.value("format_version").toString();
	if (version != "1.0")
	{
		Logger::instance().error(QString("不支持的工作流文件版本：%1").arg(version));
		return false;
	}

	graph->clear();

	// 加载节点
	QJsonArray nodesArr = root.value("nodes").toArray();
	for (const auto& nVal : nodesArr)
	{
		QJsonObject nObj = nVal.toObject();
		QString type = nObj.value("type").toString();
		QString id = nObj.value("id").toString();
		QString displayName = nObj.value("display_name").toString();
		QJsonObject posObj = nObj.value("position").toObject();
		QPointF pos(posObj.value("x").toDouble(), posObj.value("y").toDouble());

		if (!NodeFactory::instance().hasType(type))
		{
			Logger::instance().warning(QString("未知的节点类型：%1，已跳过").arg(type));
			continue;
		}

		NodeBase* node = graph->addNode(type, pos, id);
		if (!node) continue;

		node->setDisplayName(displayName);

		QJsonObject params = nObj.value("params").toObject();
		node->setAllParams(params.toVariantMap());
	}

	// 加载连线
	QJsonArray connsArr = root.value("connections").toArray();
	for (const auto& cVal : connsArr)
	{
		QJsonObject cObj = cVal.toObject();
		QString srcNode = cObj.value("from_node").toString();
		QString srcPort = cObj.value("from_port").toString();
		QString dstNode = cObj.value("to_node").toString();
		QString dstPort = cObj.value("to_port").toString();

		if (!graph->nodeById(srcNode) || !graph->nodeById(dstNode))
		{
			Logger::instance().warning(QString("连线引用了不存在的节点，已跳过"));
			continue;
		}

		if (!graph->addConnection(srcNode, srcPort, dstNode, dstPort))
		{
			Logger::instance().warning(QString("连线创建失败：%1.%2 -> %3.%4").arg(srcNode, srcPort, dstNode, dstPort));
		}
	}

	// 加载排序提示
	QJsonArray hintsArr = root.value("execution_order_hints").toArray();
	graph->setAllExecutionOrderHints(hintsArr.toVariantList());

	return true;
}
