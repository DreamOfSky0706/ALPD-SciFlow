// common/Logger.cpp
#include "Logger.h"
#include <QTextStream>
#include <iostream>

Logger& Logger::instance()
{
	static Logger s_instance;
	return s_instance;
}

Logger::Logger()
	: QObject(nullptr)
{
}

void Logger::info(const QString& msg)
{
	append(LogLevel::Info, msg);
}

void Logger::warning(const QString& msg)
{
	append(LogLevel::Warning, msg);
}

void Logger::error(const QString& msg)
{
	append(LogLevel::Error, msg);
}

void Logger::success(const QString& msg)
{
	append(LogLevel::Success, msg);
}

void Logger::clear()
{
	QMutexLocker lock(&m_mutex);
	m_entries.clear();
}

QVector<LogEntry> Logger::entries() const
{
	QMutexLocker lock(&m_mutex);
	return m_entries;
}

void Logger::setConsoleMode(bool enabled)
{
	m_consoleMode = enabled;
}

bool Logger::isConsoleMode() const
{
	return m_consoleMode;
}

void Logger::append(LogLevel level, const QString& msg)
{
	LogEntry entry;
	entry.timestamp = QDateTime::currentDateTime();
	entry.level = level;
	entry.message = msg;

	{
		QMutexLocker lock(&m_mutex);
		m_entries.append(entry);
	}

	if (m_consoleMode)
	{
		QString timeStr = entry.timestamp.toString("HH:mm:ss");
		QString prefix;
		switch (level)
		{
		case LogLevel::Info:
			prefix = "[INFO]";
			break;
		case LogLevel::Warning:
			prefix = "[WARN]";
			break;
		case LogLevel::Error:
			prefix = "[ERROR]";
			break;
		case LogLevel::Success:
			prefix = "[OK]";
			break;
		}

		QString line = QString("%1 %2 %3").arg(timeStr, prefix, msg);

		if (level == LogLevel::Error)
		{
			std::cerr << line.toLocal8Bit().constData() << std::endl;
		}
		else
		{
			std::cout << line.toLocal8Bit().constData() << std::endl;
		}
	}

	emit logAdded(entry);
}
