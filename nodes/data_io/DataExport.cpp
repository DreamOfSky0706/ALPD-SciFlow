// nodes/data_io/DataExport.cpp
#include "DataExport.h"
#include "DataTable.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include <QStringConverter>

void DataExport::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addParam("file_path","输出路径",ParamType::SaveFilePath,QString(),{{"filter","CSV文件 (*.csv);;JSON文件 (*.json)"}});
	addParam("format","格式",ParamType::Combo,QString("CSV"),{{"options",QStringList{"CSV","JSON"}}});
	addParam("delimiter","分隔符",ParamType::Combo,QString("逗号"),{{"options",QStringList{"逗号","制表符","分号"}},{"visible_when","format==CSV"}});
	addParam("encoding","编码",ParamType::Combo,QString("UTF-8"),{{"options",QStringList{"UTF-8","GBK","Latin-1"}}});
}

void DataExport::process()
{
	auto inputData=getInput("input_data");
	if(!inputData||inputData->isNull()){reportError("输入数据为空");return;}
	auto table=inputData->toDataTable();
	if(!table){reportError("无法获取DataTable数据");return;}

	QString filePath=param("file_path").toString();
	if(filePath.isEmpty()){reportError("未指定输出路径");return;}
	QFileInfo fi(filePath); QDir dir=fi.absoluteDir();
	if(!dir.exists()) dir.mkpath(".");

	QString format=param("format").toString();
	if(format=="CSV"){
		QString delimName=param("delimiter").toString();
		QChar delim=','; if(delimName=="制表符")delim='\t'; else if(delimName=="分号")delim=';';

		QString encoding=param("encoding").toString();
		QFile file(filePath);
		if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){reportError(QString("无法写入文件：%1").arg(filePath));return;}

		QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		if(encoding=="GBK")stream.setCodec("GBK");
		else if(encoding=="Latin-1")stream.setCodec("Latin-1");
		else{stream.setCodec("UTF-8");stream.setGenerateByteOrderMark(true);}
#else
		if(encoding=="GBK")stream.setEncoding(QStringConverter::System);
		else if(encoding=="Latin-1")stream.setEncoding(QStringConverter::Latin1);
		else{stream.setEncoding(QStringConverter::Utf8);stream.setGenerateByteOrderMark(true);}
#endif
		QStringList cn=table->columnNames(); stream<<cn.join(delim)<<"\n";
		for(int r=0;r<table->rowCount();++r){
			QStringList vals;
			for(int c=0;c<table->columnCount();++c){
				QVariant v=table->value(r,c);
				vals<<(v.isNull()?"":v.toString());
			}
			stream<<vals.join(delim)<<"\n";
		}
		file.close();
	}else{
		QJsonArray arr; QStringList cn=table->columnNames();
		for(int r=0;r<table->rowCount();++r){
			QJsonObject obj;
			for(int c=0;c<table->columnCount();++c){
				QVariant v=table->value(r,c);
				if(v.isNull())obj[cn[c]]=QJsonValue::Null;
				else if(v.canConvert<double>()&&v.type()!=QVariant::String)obj[cn[c]]=v.toDouble();
				else obj[cn[c]]=v.toString();
			}
			arr.append(obj);
		}
		QJsonDocument doc(arr); QFile file(filePath);
		if(!file.open(QIODevice::WriteOnly)){reportError(QString("无法写入文件：%1").arg(filePath));return;}
		file.write(doc.toJson(QJsonDocument::Indented)); file.close();
	}
}
