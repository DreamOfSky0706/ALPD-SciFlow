// nodes/utility/ExternalScript.cpp
#include "ExternalScript.h"
#include "Utility.h"
#include <opencv2/imgcodecs.hpp>
#include <QProcess>
#include <QTemporaryDir>

void ExternalScript::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("script_path", "可执行文件路径", ParamType::FilePath, QString());
	addParam("args_template", "参数模板", ParamType::String, QString("{input} {output}"),
			 { {"placeholder", "使用{input}和{output}作为占位符"} });
	addParam("timeout", "超时(秒)", ParamType::IntSlider, 30, { {"min", 1}, {"max", 600} });
}

void ExternalScript::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull())
	{
		reportError("输入图像为空"); return;
	}

	QString scriptPath = param("script_path").toString();
	if (scriptPath.isEmpty())
	{
		reportError("未指定可执行文件路径"); return;
	}

	cv::Mat src = inputData->toImage();
	QTemporaryDir tmpDir;
	if (!tmpDir.isValid())
	{
		reportError("无法创建临时目录"); return;
	}

	QString inputFile = tmpDir.filePath("input.png");
	QString outputFile = tmpDir.filePath("output.png");

	cv::imwrite(inputFile.toStdString(), src);

	QString args = param("args_template").toString();
	args.replace("{input}", inputFile);
	args.replace("{output}", outputFile);

	QProcess proc;
	proc.setProgram(scriptPath);
	proc.setArguments(args.split(" ", Qt::SkipEmptyParts));
	proc.start();

	int timeout = param("timeout").toInt() * 1000;
	if (!proc.waitForFinished(timeout))
	{
		proc.kill();
		reportError("外部脚本执行超时");
		return;
	}

	if (proc.exitCode() != 0)
	{
		QString errOutput = proc.readAllStandardError();
		reportError(QString("外部脚本返回非零退出码：%1\n%2").arg(proc.exitCode()).arg(errOutput));
		return;
	}

	cv::Mat result = cv::imread(outputFile.toStdString(), cv::IMREAD_UNCHANGED);
	if (result.empty())
	{
		reportError("无法读取外部脚本的输出文件"); return;
	}

	setOutput("output_image", NodeData::createImage(result));
}
