// io/DataIO.cpp
#include "DataIO.h"
#include "DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QStringConverter>

std::shared_ptr<DataTable> DataIO::readCSV(const QString& filePath, QChar delimiter, bool hasHeader, const QString& encoding)
{
	auto table = std::make_shared<DataTable>();
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return table;

	QTextStream stream(&file);
	// Qt6 QTextStream 默认UTF-8且自动跳过BOM，仅非UTF-8时显式设置
	if (encoding == "GBK")
		stream.setEncoding(QStringConverter::System);
	else if (encoding == "Latin-1")
		stream.setEncoding(QStringConverter::Latin1);
	// UTF-8: 不设置，保持默认避免BOM污染首列名
	bool first = true;
	while (!stream.atEnd())
	{
		QString line = stream.readLine().trimmed();
		if (line.isEmpty()) continue;
		QStringList fields = line.split(delimiter);
		if (first && hasHeader)
		{
			for (const auto& f : fields) table->addColumn(f.trimmed());
			first = false;
			continue;
		}
		if (first)
		{
			for (int i = 0; i < fields.size(); ++i) table->addColumn(QString("Column_%1").arg(i));
			first = false;
		}
		QVariantList row;
		for (const auto& f : fields)
		{
			bool ok; double v = f.trimmed().toDouble(&ok);
			row << (ok ? QVariant(v) : QVariant(f.trimmed()));
		}
		table->addRow(row);
	}
	file.close();
	return table;
}

bool DataIO::writeCSV(const QString& filePath, const std::shared_ptr<DataTable>& table, QChar delimiter)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
	QTextStream stream(&file);
	stream.setEncoding(QStringConverter::Utf8);
	stream.setGenerateByteOrderMark(true);
	stream << table->columnNames().join(delimiter) << "\n";
	for (int r = 0; r < table->rowCount(); ++r)
	{
		QStringList vals;
		for (int c = 0; c < table->columnCount(); ++c)
			vals << table->value(r, c).toString();
		stream << vals.join(delimiter) << "\n";
	}
	file.close();
	return true;
}
