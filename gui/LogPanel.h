#ifndef SCIFLOW_LOGPANEL_H
#define SCIFLOW_LOGPANEL_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include "../common/Logger.h"

class LogPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LogPanel(QWidget *parent = nullptr);
    ~LogPanel() override;

private slots:
    void onLogAdded(const LogEntry &entry);
    void onClearClicked();
    void onExportClicked();
    void onFilterChanged(int index);

private:
    void setupUI();
    void applyDarkTheme();
    void rerenderLogs();
    bool passesFilter(const LogEntry &entry) const;

    QTextEdit   *m_logView;
    QPushButton *m_clearBtn;
    QPushButton *m_exportBtn;
    QComboBox   *m_logFilter;

    QVector<LogEntry> m_logEntries;
    int m_currentFilter = -1;  // -1 = 全部, 0=Info, 1=Warning, 2=Error, 3=Success
};

#endif // SCIFLOW_LOGPANEL_H
