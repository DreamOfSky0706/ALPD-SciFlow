// cli/CLIRunner.cpp
#include "CLIRunner.h"
#include "WorkflowGraph.h"
#include "WorkflowSerializer.h"
#include "Logger.h"
#include "RegisterNodes.h"
#include <QCoreApplication>
#include <iostream>

// 输出中文到Windows控制台（转本地编码）
static void cout(const QString& s) { std::cout << s.toLocal8Bit().constData() << std::endl; }
static void cerr(const QString& s) { std::cerr << s.toLocal8Bit().constData() << std::endl; }

int CLIRunner::run(int argc, char* argv[])
{
	QStringList args;
	for (int i = 1; i < argc; ++i) args << QString::fromLocal8Bit(argv[i]);

	if (args.contains("--help") || args.contains("-h")) { printHelp(); return 0; }

	if (args.contains("--version")) { std::cout << "ImageNodeTool 1.0.0" << std::endl; return 0; }

	int wfIdx = args.indexOf("--workflow");
	if (wfIdx < 0 || wfIdx + 1 >= args.size()) {
		cerr(QStringLiteral("错误：未指定工作流文件，使用 --workflow <文件路径>"));
		return 2;
	}

	QString workflowPath = args[wfIdx + 1];
	Logger::instance().setConsoleMode(true);

	WorkflowGraph graph;
	if (!WorkflowSerializer::load(workflowPath, &graph)) {
		cerr(QStringLiteral("错误：加载工作流文件失败"));
		return 2;
	}

	for (int i = 0; i < args.size(); ++i) {
		if (args[i] != "--param" || i + 1 >= args.size()) continue;
		QString paramStr = args[++i];
		int dotPos = paramStr.indexOf('.');
		int eqPos = paramStr.indexOf('=');
		if (dotPos < 0 || eqPos < 0 || eqPos <= dotPos) {
			Logger::instance().warning(QString("无效的参数覆盖格式：%1").arg(paramStr));
			continue;
		}
		QString nodeId = paramStr.left(dotPos);
		QString paramName = paramStr.mid(dotPos + 1, eqPos - dotPos - 1);
		QString value = paramStr.mid(eqPos + 1);
		NodeBase* node = graph.nodeById(nodeId);
		if (!node) { Logger::instance().warning(QString("参数覆盖：节点[%1]不存在").arg(nodeId)); continue; }
		graph.changeParam(nodeId, paramName, value);
	}

	graph.executeAll();
	return graph.lastFailCount() > 0 ? 1 : 0;
}

void CLIRunner::printHelp()
{
	cout(QStringLiteral("用法：ImageNodeTool [选项]"));
	cout(QStringLiteral(""));
	cout(QStringLiteral("选项："));
	cout(QStringLiteral("  --workflow <文件路径>           指定工作流文件 (.flow.json)"));
	cout(QStringLiteral("  --no-gui                       命令行模式运行"));
	cout(QStringLiteral("  --param <节点ID>.<参数名>=<值>  覆盖参数（可多次使用）"));
	cout(QStringLiteral("  --help                         显示帮助信息"));
	cout(QStringLiteral("  --version                      显示版本号"));
}
