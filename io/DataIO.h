// io/DataIO.h
#pragma once

#include <QString>
#include <memory>

class DataTable;

class DataIO
{
public:
	static std::shared_ptr<DataTable> readCSV(const QString& filePath, QChar delimiter = ',', bool hasHeader = true, const QString& encoding = "UTF-8");
	static bool writeCSV(const QString& filePath, const std::shared_ptr<DataTable>& table, QChar delimiter = ',');
};
