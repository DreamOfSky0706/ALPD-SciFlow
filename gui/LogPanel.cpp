#include "LogPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFont>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QDateTime>

// ===================================================================
// LogPanel 实现
// ===================================================================

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
    , m_logView(nullptr)
    , m_clearBtn(nullptr)
    , m_exportBtn(nullptr)
    , m_logFilter(nullptr)
    , m_currentFilter(-1)
{
    setupUI();
    applyDarkTheme();

    // 连接 Logger 单例的日志信号
    connect(&Logger::instance(), &Logger::logAdded,
            this, &LogPanel::onLogAdded);
}

LogPanel::~LogPanel()
{
    disconnect(&Logger::instance(), &Logger::logAdded,
               this, &LogPanel::onLogAdded);
}

// -------------------------------------------------------------------
// UI 构建
// -------------------------------------------------------------------

void LogPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // ---- 顶部按钮栏 ----
    auto *buttonBar = new QHBoxLayout();

    // 日志级别过滤器
    m_logFilter = new QComboBox(this);
    m_logFilter->addItem(QStringLiteral("全部"), -1);
    m_logFilter->addItem(QStringLiteral("INFO"), 0);
    m_logFilter->addItem(QStringLiteral("WARNING"), 1);
    m_logFilter->addItem(QStringLiteral("ERROR"), 2);
    m_logFilter->addItem(QStringLiteral("SUCCESS"), 3);
    m_logFilter->setCurrentIndex(0);
    m_logFilter->setFixedWidth(100);
    connect(m_logFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogPanel::onFilterChanged);
    buttonBar->addWidget(m_logFilter);

    buttonBar->addStretch();

    m_clearBtn = new QPushButton(QStringLiteral("清除"), this);
    m_clearBtn->setFixedSize(60, 24);
    connect(m_clearBtn, &QPushButton::clicked,
            this, &LogPanel::onClearClicked);
    buttonBar->addWidget(m_clearBtn);

    m_exportBtn = new QPushButton(QStringLiteral("导出日志"), this);
    m_exportBtn->setFixedSize(72, 24);
    connect(m_exportBtn, &QPushButton::clicked,
            this, &LogPanel::onExportClicked);
    buttonBar->addWidget(m_exportBtn);

    mainLayout->addLayout(buttonBar);

    // ---- 日志显示区 ----
    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setUndoRedoEnabled(false);

    // 等宽字体，10pt
    QFont monoFont(QStringLiteral("Consolas"), 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_logView->setFont(monoFont);

    mainLayout->addWidget(m_logView, 1);
}

// -------------------------------------------------------------------
// 暗色主题
// -------------------------------------------------------------------

void LogPanel::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "LogPanel {"
        "  background-color: #2b2b2b;"
        "  color: #e0e0e0;"
        "}"
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 2px;"
        "  selection-background-color: #3a5a4a;"
        "}"
        "QPushButton {"
        "  background-color: #3a3a3a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #4a4a4a;"
        "  border-radius: 3px;"
        "  padding: 2px 8px;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #4a4a4a;"
        "  border-color: #5a5a5a;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #333333;"
        "}"
        "QComboBox {"
        "  background-color: #3a3a3a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #4a4a4a;"
        "  border-radius: 3px;"
        "  padding: 2px 6px;"
        "  font-size: 11px;"
        "}"
        "QComboBox::drop-down {"
        "  background-color: #404040;"
        "  border: none;"
        "  width: 18px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #3a3a3a;"
        "  color: #e0e0e0;"
        "  selection-background-color: #4EC9B0;"
        "}"
    ));
}

// -------------------------------------------------------------------
// 过滤器判断
// -------------------------------------------------------------------

bool LogPanel::passesFilter(const LogEntry &entry) const
{
    if (m_currentFilter < 0)
        return true;  // "全部"：不过滤
    return (int)entry.level >= m_currentFilter;
}

// -------------------------------------------------------------------
// 日志追加
// -------------------------------------------------------------------

void LogPanel::onLogAdded(const LogEntry &entry)
{
    // 始终存储到完整列表
    m_logEntries.append(entry);

    // 如果当前过滤器不通过，不显示
    if (!passesFilter(entry))
        return;

    // 根据日志级别确定颜色和标签
    QColor color;
    QString levelStr;

    switch (entry.level) {
    case LogLevel::Info:
        color    = QColor(QStringLiteral("#a0a0a0"));
        levelStr = QStringLiteral("INFO");
        break;
    case LogLevel::Warning:
        color    = QColor(QStringLiteral("#e0c040"));
        levelStr = QStringLiteral("WARNING");
        break;
    case LogLevel::Error:
        color    = QColor(QStringLiteral("#e04040"));
        levelStr = QStringLiteral("ERROR");
        break;
    case LogLevel::Success:
        color    = QColor(QStringLiteral("#40c040"));
        levelStr = QStringLiteral("SUCCESS");
        break;
    }

    // 组装格式化行：HH:mm:ss [LEVEL] message
    QString line = QStringLiteral("%1 [%2] %3\n")
                       .arg(entry.timestamp.toString(QStringLiteral("HH:mm:ss")))
                       .arg(levelStr)
                       .arg(entry.message);

    // 使用 QTextCursor 追加带颜色的文本
    QTextCursor cursor = m_logView->textCursor();
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat fmt;
    fmt.setForeground(color);
    cursor.insertText(line, fmt);

    // 自动滚动到底部
    QScrollBar *sb = m_logView->verticalScrollBar();
    if (sb)
        sb->setValue(sb->maximum());
}

// -------------------------------------------------------------------
// 过滤器变化：重新渲染所有日志
// -------------------------------------------------------------------

void LogPanel::onFilterChanged(int index)
{
    m_currentFilter = m_logFilter->itemData(index).toInt();
    rerenderLogs();
}

void LogPanel::rerenderLogs()
{
    m_logView->clear();

    for (const auto &entry : m_logEntries) {
        if (!passesFilter(entry))
            continue;

        QColor color;
        QString levelStr;

        switch (entry.level) {
        case LogLevel::Info:
            color    = QColor(QStringLiteral("#a0a0a0"));
            levelStr = QStringLiteral("INFO");
            break;
        case LogLevel::Warning:
            color    = QColor(QStringLiteral("#e0c040"));
            levelStr = QStringLiteral("WARNING");
            break;
        case LogLevel::Error:
            color    = QColor(QStringLiteral("#e04040"));
            levelStr = QStringLiteral("ERROR");
            break;
        case LogLevel::Success:
            color    = QColor(QStringLiteral("#40c040"));
            levelStr = QStringLiteral("SUCCESS");
            break;
        }

        QString line = QStringLiteral("%1 [%2] %3\n")
                           .arg(entry.timestamp.toString(QStringLiteral("HH:mm:ss")))
                           .arg(levelStr)
                           .arg(entry.message);

        QTextCursor cursor = m_logView->textCursor();
        cursor.movePosition(QTextCursor::End);

        QTextCharFormat fmt;
        fmt.setForeground(color);
        cursor.insertText(line, fmt);
    }

    // 自动滚动到底部
    QScrollBar *sb = m_logView->verticalScrollBar();
    if (sb)
        sb->setValue(sb->maximum());
}

// -------------------------------------------------------------------
// 清除
// -------------------------------------------------------------------

void LogPanel::onClearClicked()
{
    Logger::instance().clear();
    m_logView->clear();
    m_logEntries.clear();
}

// -------------------------------------------------------------------
// 导出
// -------------------------------------------------------------------

void LogPanel::onExportClicked()
{
    QString defaultName = QString("sciflow_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString filePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("导出日志"),
        defaultName,
        QStringLiteral("Text Files (*.txt);;All Files (*)"));

    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    // Qt6 默认使用 UTF-8 编码

    const auto &entries = Logger::instance().entries();
    for (const auto &entry : entries) {
        QString levelStr;
        switch (entry.level) {
        case LogLevel::Info:    levelStr = QStringLiteral("INFO");    break;
        case LogLevel::Warning: levelStr = QStringLiteral("WARNING"); break;
        case LogLevel::Error:   levelStr = QStringLiteral("ERROR");   break;
        case LogLevel::Success: levelStr = QStringLiteral("SUCCESS"); break;
        }

        stream << entry.timestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
               << " [" << levelStr << "] "
               << entry.message << "\n";
    }

    file.close();
}
