// nodes/text_io/TextFileRead.cpp
#include "TextFileRead.h"
#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringConverter>

void TextFileRead::defineNode()
{
    addOutputPort("output_text", DataType::Text);

    addParam("file_path", "文件路径", ParamType::FilePath, QString(),
             {{"filter", "文本文件 (*.txt);;所有文件 (*)"}});
    addParam("encoding", "编码", ParamType::Combo, QString("UTF-8"),
             {{"options", QStringList{"UTF-8", "GBK", "Latin-1"}}});
}

void TextFileRead::process()
{
    QString path = param("file_path").toString();
    if (path.isEmpty()) {
        reportError("未指定输入文件路径");
        return;
    }
    QFileInfo fi(path);
    if (!fi.exists()) {
        reportError(QString("文件不存在: %1").arg(path));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        reportError(QString("无法读取文件: %1").arg(path));
        return;
    }

    QTextStream stream(&file);
    QString enc = param("encoding").toString();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (enc == "GBK") stream.setCodec("GBK");
    else if (enc == "Latin-1") stream.setCodec("Latin-1");
    else stream.setCodec("UTF-8");
#else
    if (enc == "GBK") stream.setEncoding(QStringConverter::System);
    else stream.setEncoding(QStringConverter::Utf8);
#endif

    QString content = stream.readAll();
    file.close();

    Logger::instance().info(QString("读取文本文件: %1 (%2字符)").arg(path).arg(content.size()));
    setOutput("output_text", NodeData::createText(content));
}
