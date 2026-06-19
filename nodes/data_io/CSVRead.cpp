// nodes/data_io/CSVRead.cpp
#include "CSVRead.h"
#include "DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QStringDecoder>

void CSVRead::defineNode()
{
	addOutputPort("output_data", DataType::DataTable);

	addParam("file_path", "文件路径", ParamType::FilePath, QString(),
			 { {"filter", "CSV文件 (*.csv);;所有文件 (*.*)"} });
	addParam("delimiter", "分隔符", ParamType::Combo, QString("逗号"),
			 { {"options", QStringList{"逗号", "制表符", "分号", "空格", "自定义"}} });
	addParam("custom_delimiter", "自定义分隔符", ParamType::String, QString(),
			 { {"visible_when", "delimiter==自定义"} });
	addParam("has_header", "首行为列名", ParamType::Bool, true);
	addParam("encoding", "编码", ParamType::Combo, QString("UTF-8"),
			 { {"options", QStringList{"UTF-8", "GBK", "Latin-1"}} });
}

void CSVRead::process()
{
	QString filePath = param("file_path").toString();
	if (filePath.isEmpty())
	{
		reportError("未指定CSV文件路径");
		return;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		reportError(QString("无法打开文件：%1").arg(filePath));
		return;
	}

	// 确定分隔符
	QString delimName = param("delimiter").toString();
	QChar delim = ',';
	if (delimName == "制表符") delim = '\t';
	else if (delimName == "分号") delim = ';';
	else if (delimName == "空格") delim = ' ';
	else if (delimName == "自定义")
	{
		QString customD = param("custom_delimiter").toString();
		if (!customD.isEmpty()) delim = customD[0];
	}

	// 编码：Qt6 QTextStream 默认UTF-8自动检测BOM，仅非UTF-8时需显式设置
	QString encoding = param("encoding").toString();
	QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	if (encoding == "GBK") stream.setCodec("GBK");
	else if (encoding == "Latin-1") stream.setCodec("Latin-1");
	else stream.setCodec("UTF-8");
#else
	// UTF-8 是Qt6默认编码，且 QTextStream 会自动跳过BOM，不需显式设置
	// 显式 setEncoding 会重置流导致BOM被读入数据污染首列名
	if (encoding == "GBK")
		stream.setEncoding(QStringConverter::System);
	else if (encoding == "Latin-1")
		stream.setEncoding(QStringConverter::Latin1);
	// UTF-8: 不设置，保持默认（已自动处理BOM）
#endif

	bool hasHeader = param("has_header").toBool();
	auto table = std::make_shared<DataTable>();

	bool firstLine = true;
	while (!stream.atEnd())
	{
		QString line = stream.readLine().trimmed();
		if (line.isEmpty()) continue;

		QStringList fields = line.split(delim);

		if (firstLine)
		{
			if (hasHeader)
			{
				for (const auto& f : fields)
				{
					table->addColumn(f.trimmed());
				}
				firstLine = false;
				continue;
			}
			else
			{
				for (int i = 0; i < fields.size(); ++i)
				{
					table->addColumn(QString("Column_%1").arg(i));
				}
			}
			firstLine = false;
		}

		QVariantList rowValues;
		for (const auto& f : fields)
		{
			QString trimmed = f.trimmed();
			bool ok;
			double num = trimmed.toDouble(&ok);
			if (ok)
			{
				rowValues << num;
			}
			else
			{
				rowValues << trimmed;
			}
		}
		table->addRow(rowValues);
	}

	file.close();
	setOutput("output_data", NodeData::createDataTable(table));
}
