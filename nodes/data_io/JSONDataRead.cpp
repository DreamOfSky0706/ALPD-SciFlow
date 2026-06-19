// nodes/data_io/JSONDataRead.cpp
#include "JSONDataRead.h"
#include "DataTable.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

void JSONDataRead::defineNode()
{
	addOutputPort("output_data", DataType::DataTable);

	addParam("file_path", "文件路径", ParamType::FilePath, QString(),
			 { {"filter", "JSON文件 (*.json)"} });
	addParam("root_path", "数据路径", ParamType::String, QString(),
			 { {"placeholder", "如 data.records，空表示根节点"} });
}

void JSONDataRead::process()
{
	QString filePath = param("file_path").toString();
	if (filePath.isEmpty())
	{
		reportError("未指定JSON文件路径");
		return;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly))
	{
		reportError(QString("无法打开文件：%1").arg(filePath));
		return;
	}

	QByteArray content = file.readAll();
	file.close();

	QJsonParseError parseError;
	QJsonDocument doc = QJsonDocument::fromJson(content, &parseError);
	if (parseError.error != QJsonParseError::NoError)
	{
		reportError(QString("JSON解析错误：%1").arg(parseError.errorString()));
		return;
	}

	// 导航到指定路径
	QString rootPath = param("root_path").toString().trimmed();
	QJsonValue current;

	if (doc.isArray())
	{
		current = QJsonValue(doc.array());
	}
	else
	{
		current = QJsonValue(doc.object());
	}

	if (!rootPath.isEmpty())
	{
		QStringList keys = rootPath.split(".");
		for (const auto& key : keys)
		{
			if (current.isObject())
			{
				current = current.toObject().value(key);
			}
			else
			{
				reportError(QString("路径[%1]无效，中间节点不是对象").arg(rootPath));
				return;
			}
		}
	}

	if (!current.isArray())
	{
		reportError("指定路径下的数据不是数组");
		return;
	}

	QJsonArray arr = current.toArray();
	if (arr.isEmpty())
	{
		reportWarning("JSON数组为空");
		setOutput("output_data", NodeData::createDataTable(std::make_shared<DataTable>()));
		return;
	}

	// 收集所有列名
	QStringList allKeys;
	for (const auto& elem : arr)
	{
		if (elem.isObject())
		{
			QJsonObject obj = elem.toObject();
			for (const auto& key : obj.keys())
			{
				if (!allKeys.contains(key))
				{
					allKeys.append(key);
				}
			}
		}
	}

	auto table = std::make_shared<DataTable>();
	for (const auto& key : allKeys)
	{
		table->addColumn(key);
	}

	// 填充数据
	for (const auto& elem : arr)
	{
		QVariantList rowValues;
		if (elem.isObject())
		{
			QJsonObject obj = elem.toObject();
			for (const auto& key : allKeys)
			{
				QJsonValue val = obj.value(key);
				if (val.isDouble())
				{
					rowValues << val.toDouble();
				}
				else if (val.isString())
				{
					rowValues << val.toString();
				}
				else if (val.isBool())
				{
					rowValues << val.toBool();
				}
				else
				{
					rowValues << QVariant();
				}
			}
		}
		else
		{
			for (int i = 0; i < allKeys.size(); ++i)
			{
				rowValues << QVariant();
			}
		}
		table->addRow(rowValues);
	}

	setOutput("output_data", NodeData::createDataTable(table));
}
