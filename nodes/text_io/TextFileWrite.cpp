// nodes/text_io/TextFileWrite.cpp
#include "TextFileWrite.h"
#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

void TextFileWrite::defineNode()
{
    addInputPort("input_text", DataType::Text);

    addParam("file_path", "输出路径", ParamType::SaveFilePath,
             QString(QDir::currentPath() + "/output.txt"),
             {{"filter", "文本文件 (*.txt);;所有文件 (*)"}});
    addParam("encoding", "编码", ParamType::Combo, QString("UTF-8"),
             {{"options", QStringList{"UTF-8", "GBK", "Latin-1"}}});
}

void TextFileWrite::process()
{
    auto data = getInput("input_text");
    if (!data || data->isNull()) {
        reportError("输入文本为空");
        return;
    }

    QString text = data->toText();
    QString path = param("file_path").toString();
    if (path.isEmpty()) {
        reportError("未指定输出路径");
        return;
    }

    QFileInfo fi(path);
    QDir dir = fi.absoluteDir();
    if (!dir.exists()) dir.mkpath(".");

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        reportError(QString("无法写入文件: %1").arg(path));
        return;
    }

    QString enc = param("encoding").toString();
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (enc == "GBK") stream.setCodec("GBK");
    else if (enc == "Latin-1") stream.setCodec("Latin-1");
    else { stream.setCodec("UTF-8"); stream.setGenerateByteOrderMark(true); }
#else
    if (enc == "GBK") stream.setEncoding(QStringConverter::System);
    else if (enc == "Latin-1") stream.setEncoding(QStringConverter::Latin1);
    else { stream.setEncoding(QStringConverter::Utf8); stream.setGenerateByteOrderMark(true); }
#endif
    stream << text;
    file.close();

    Logger::instance().success(QString("文本已导出: %1 (%2字符)").arg(path).arg(text.size()));
}
