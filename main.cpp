// main.cpp
#include <QApplication>
#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QPixmapCache>
#include "RegisterNodes.h"
#include "CLIRunner.h"
#include "Logger.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
	// 检测是否为命令行模式
	bool cliMode = false;
	for (int i = 1; i < argc; ++i)
	{
		if (QString::fromLocal8Bit(argv[i]) == "--no-gui")
		{
			cliMode = true;
			break;
		}
	}

	// 注册所有节点
	registerAllNodes();

	if (cliMode)
	{
		QCoreApplication app(argc, argv);
		app.setApplicationName("SciFlow");
		app.setApplicationVersion("1.0.0");

		Logger::instance().setConsoleMode(true);
		CLIRunner runner;
		return runner.run(argc, argv);
	}
	else
	{
		QApplication app(argc, argv);
		app.setApplicationName("SciFlow");
		app.setApplicationVersion("1.0.0");

		// 增大 QPixmapCache 限制，解决 CacheOverflowException
		QPixmapCache::setCacheLimit(65536);

		// 加载样式表（优先从构建目录加载，其次从资源文件加载）
		QString stylePath = QDir(QApplication::applicationDirPath())
			.filePath("resources/style.qss");
		QFile styleFile(stylePath);
		if (!styleFile.exists()) {
			styleFile.setFileName(":/resources/style.qss");
		}
		if (styleFile.open(QIODevice::ReadOnly))
		{
			app.setStyleSheet(styleFile.readAll());
			styleFile.close();
		}

		MainWindow mainWindow;
		mainWindow.show();

		Logger::instance().info("SciFlow GUI 模式启动完成");
		return app.exec();
	}
}
