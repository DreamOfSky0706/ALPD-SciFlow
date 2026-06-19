// nodes/data_io/ExcelRead.cpp
#include "ExcelRead.h"
#include "DataTable.h"
#include "xlsxdocument.h"
#include "xlsxcellrange.h"

void ExcelRead::defineNode()
{
	addOutputPort("output_data", DataType::DataTable);

	addParam("file_path", "文件路径", ParamType::FilePath, QString(),
			 { {"filter", "Excel文件 (*.xlsx)"} });
	addParam("sheet", "工作表", ParamType::String, QString());
	addParam("has_header", "首行为列名", ParamType::Bool, true);
}

void ExcelRead::process()
{
	QString filePath = param("file_path").toString();
	if (filePath.isEmpty())
	{
		reportError("未指定Excel文件路径");
		return;
	}

	QXlsx::Document xlsx(filePath);
	if (!xlsx.load())
	{
		reportError(QString("无法打开Excel文件：%1").arg(filePath));
		return;
	}

	QString sheetName = param("sheet").toString();
	if (!sheetName.isEmpty())
	{
		if (!xlsx.selectSheet(sheetName))
		{
			reportWarning(QString("工作表[%1]不存在，使用第一个工作表").arg(sheetName));
		}
	}

	QXlsx::CellRange range = xlsx.dimension();
	if (!range.isValid())
	{
		reportError("Excel文件中没有数据");
		return;
	}

	int firstRow = range.firstRow();
	int lastRow = range.lastRow();
	int firstCol = range.firstColumn();
	int lastCol = range.lastColumn();

	bool hasHeader = param("has_header").toBool();
	auto table = std::make_shared<DataTable>();

	// 读取列名
	if (hasHeader)
	{
		for (int c = firstCol; c <= lastCol; ++c)
		{
			QVariant val = xlsx.read(firstRow, c);
			QString colName = val.isNull() ? QString("Column_%1").arg(c - firstCol) : val.toString();
			table->addColumn(colName);
		}
		firstRow++;
	}
	else
	{
		for (int c = firstCol; c <= lastCol; ++c)
		{
			table->addColumn(QString("Column_%1").arg(c - firstCol));
		}
	}

	// 读取数据行
	for (int r = firstRow; r <= lastRow; ++r)
	{
		QVariantList rowValues;
		for (int c = firstCol; c <= lastCol; ++c)
		{
			QVariant val = xlsx.read(r, c);
			if (val.isNull())
			{
				rowValues << QVariant();
			}
			else if (val.canConvert<double>())
			{
				rowValues << val.toDouble();
			}
			else
			{
				rowValues << val.toString();
			}
		}
		table->addRow(rowValues);
	}

	setOutput("output_data", NodeData::createDataTable(table));
}
