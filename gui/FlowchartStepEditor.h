#pragma once

#include <QWidget>
#include <QVariantList>
#include <QVariantMap>
#include <QColor>
#include <QTabWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>

// 流程图步骤编辑器 —— 用于 FlowchartAutoLayout 节点
// 管理步骤定义、连接、全局样式，包含预览区
class FlowchartStepEditor : public QWidget
{
    Q_OBJECT

public:
    explicit FlowchartStepEditor(QWidget *parent = nullptr);

    void setSteps(const QVariantList &steps);
    QVariantList steps() const;

    void setGlobalStyle(const QVariantMap &style);
    QVariantMap globalStyle() const;

signals:
    void valueChanged();

private slots:
    void onAddStep();
    void onDeleteStep(int index);
    void onStepChanged();
    void onGlobalStyleChanged();
    void onTabChanged(int index);
    void onStepOrderChanged();

private:
    // 单个步骤的控件组
    QWidget *createStepWidget(int index, const QVariantMap &data);
    void rebuildStepList();
    void updatePreview();
    void onCurrentStepChanged(int row);
    void saveCurrentStep();

    // 逻辑类型 → 建议形状映射
    static QString suggestedShape(const QString &logicType);

    // 全局样式控件
    QWidget *createGlobalStylePanel();

    QTabWidget *m_tabWidget = nullptr;

    // "手动编辑" 标签页控件
    QScrollArea *m_stepScrollArea = nullptr;
    QWidget *m_stepContainer = nullptr;
    QVBoxLayout *m_stepLayout = nullptr;
    QPushButton *m_addStepButton = nullptr;
    QListWidget *m_stepListWidget = nullptr;
    QStackedWidget *m_stepDetailWidget = nullptr;

    // 全局样式控件引用
    class QComboBox *m_colorSchemeCombo = nullptr;
    class QPushButton *m_borderColorBtn = nullptr;
    class QSpinBox *m_fontSizeSpin = nullptr;
    class QSpinBox *m_canvasWidthSpin = nullptr;
    class QSpinBox *m_canvasHeightSpin = nullptr;
    class QSpinBox *m_spacingSpin = nullptr;
    class QComboBox *m_directionCombo = nullptr;
    class QComboBox *m_connectorStyleCombo = nullptr;
    QColor m_borderColor;

    // 预览区
    class QLabel *m_previewLabel = nullptr;

    QVariantList m_steps;
    QVariantMap m_globalStyle;
};
