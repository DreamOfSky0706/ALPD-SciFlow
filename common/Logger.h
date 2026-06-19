// common/Logger.h
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QMutex>

// 日志级别
enum class LogLevel
{
	Info,
	Warning,
	Error,
	Success
};

// 单条日志记录
struct LogEntry
{
	QDateTime timestamp;
	LogLevel level;
	QString message;
};

// 全局日志系统，支持GUI和CLI两种输出模式
class Logger : public QObject
{
	Q_OBJECT

public:
	static Logger& instance();

	void info(const QString& msg);
	void warning(const QString& msg);
	void error(const QString& msg);
	void success(const QString& msg);

	void clear();
	QVector<LogEntry> entries() const;

	// CLI模式下设为true，日志直接输出到stdout/stderr
	void setConsoleMode(bool enabled);
	bool isConsoleMode() const;

signals:
	// GUI模式下通过此信号通知日志面板
	void logAdded(const LogEntry& entry);

private:
	Logger();
	~Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	void append(LogLevel level, const QString& msg);

	QVector<LogEntry> m_entries;
	bool m_consoleMode = false;
	mutable QMutex m_mutex;
};
