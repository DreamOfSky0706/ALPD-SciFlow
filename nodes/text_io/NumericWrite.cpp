// nodes/text_io/NumericWrite.cpp
#include "NumericWrite.h"
#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

void NumericWrite::defineNode()
{
    addInputPort("input_value", DataType::Numeric);

    addParam("file_path", "输出路径", ParamType::SaveFilePath,
             QString(QDir::currentPath() + "/output.txt"),
             {{"filter", "文本文件 (*.txt);;所有文件 (*)"}});
    addParam("precision", "小数位数", ParamType::Int, 6,
             {{"min", 0}, {"max", 15}});
}

void NumericWrite::process()
{
    auto data = getInput("input_value");
    if (!data || data->isNull()) {
        reportError("输入数值为空");
        return;
    }

    double value = data->toNumeric();
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

    int prec = param("precision").toInt();
    QTextStream stream(&file);
    stream << QString::number(value, 'f', prec);
    file.close();

    Logger::instance().success(QString("数值已导出: %1 = %2").arg(path)
        .arg(QString::number(value, 'f', prec)));
}
