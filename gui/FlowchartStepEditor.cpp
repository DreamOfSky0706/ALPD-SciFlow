#include "FlowchartStepEditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QColorDialog>
#include <QPainter>
#include <QFontMetrics>
#include <QListWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QtMath>

// ---- 辅助：暗色主题样式表 ----
static const QString kDarkStyle = R"(
    QGroupBox {
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        border-radius: 4px;
        margin-top: 8px;
        padding-top: 14px;
        background-color: #2b2b2b;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 4px;
        color: #4EC9B0;
    }
    QComboBox {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 2px 6px;
        border-radius: 2px;
        min-width: 100px;
    }
    QComboBox::drop-down { border: none; }
    QComboBox QAbstractItemView {
        background-color: #3a3a3a;
        color: #e0e0e0;
        selection-background-color: #4EC9B0;
        selection-color: #1e1e1e;
    }
    QSpinBox {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 2px 4px;
        border-radius: 2px;
    }
    QLineEdit {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 3px 6px;
        border-radius: 2px;
    }
    QPushButton {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 4px 12px;
        border-radius: 2px;
    }
    QPushButton:hover { background-color: #4a4a4a; }
    QPushButton#addStepBtn {
        background-color: #4EC9B0;
        color: #1e1e1e;
        font-weight: bold;
    }
    QPushButton#addStepBtn:hover { background-color: #5fd9c0; }
    QPushButton#delStepBtn {
        background-color: #6e3030;
        color: #e0e0e0;
    }
    QPushButton#delStepBtn:hover { background-color: #8e4040; }
    QPushButton#colorBtn {
        min-width: 32px;
        min-height: 22px;
        border: 1px solid #4a4a4a;
        border-radius: 2px;
    }
    QScrollArea {
        border: none;
        background-color: #2b2b2b;
    }
    QListWidget {
        background-color: #2b2b2b;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
    }
    QListWidget::item:selected {
        background-color: #4EC9B0;
        color: #1e1e1e;
    }
    QTabWidget::pane {
        border: 1px solid #4a4a4a;
        background-color: #2b2b2b;
    }
    QTabBar::tab {
        background-color: #3a3a3a;
        color: #e0e0e0;
        padding: 6px 16px;
        border: 1px solid #4a4a4a;
        border-bottom: none;
        border-top-left-radius: 4px;
        border-top-right-radius: 4px;
    }
    QTabBar::tab:selected {
        background-color: #2b2b2b;
        border-bottom: 2px solid #4EC9B0;
    }
    QLabel {
        color: #e0e0e0;
    }
)";

// ---- 逻辑类型 → 建议形状 ----
QString FlowchartStepEditor::suggestedShape(const QString &logicType)
{
    if (logicType == "Start" || logicType == "End")
        return "RoundedRect";
    if (logicType == "Process" || logicType == "I-O")
        return "Rect";
    if (logicType == "Decision")
        return "Diamond";
    if (logicType == "Subprocess")
        return "Rect";
    if (logicType == "Wait")
        return "RoundedRect";
    if (logicType == "ParallelStart" || logicType == "ParallelEnd")
        return "Rect";
    return "Rect";
}

FlowchartStepEditor::FlowchartStepEditor(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(280, 550);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(4);

    // ---- 双标签页：步骤编辑 | 全局样式 ----
    m_tabWidget = new QTabWidget;
    m_tabWidget->setStyleSheet(kDarkStyle);

    // ==== 标签1: 步骤编辑 ====
    QWidget *editTab = new QWidget;
    QVBoxLayout *editLayout = new QVBoxLayout(editTab);
    editLayout->setContentsMargins(2, 2, 2, 2);
    editLayout->setSpacing(2);

    // 步骤列表（紧凑）
    m_stepListWidget = new QListWidget;
    m_stepListWidget->setMaximumHeight(60);
    m_stepListWidget->setStyleSheet(kDarkStyle);
    connect(m_stepListWidget, &QListWidget::currentRowChanged, this, &FlowchartStepEditor::onCurrentStepChanged);
    editLayout->addWidget(new QLabel("步骤列表(点击选中可编辑下方详情):"));
    editLayout->addWidget(m_stepListWidget);

    // 步骤详情区（只显示选中步骤）
    m_stepScrollArea = new QScrollArea;
    m_stepScrollArea->setWidgetResizable(true);
    m_stepScrollArea->setStyleSheet(kDarkStyle);
    m_stepContainer = new QWidget;
    m_stepLayout = new QVBoxLayout(m_stepContainer);
    m_stepLayout->setContentsMargins(2, 2, 2, 2);
    m_stepLayout->setSpacing(6);
    m_stepDetailWidget = new QStackedWidget;
    m_stepLayout->addWidget(m_stepDetailWidget);
    m_stepLayout->addStretch();
    m_stepScrollArea->setWidget(m_stepContainer);
    editLayout->addWidget(m_stepScrollArea, 1);

    // 底部操作按钮行
    QHBoxLayout *btnRow = new QHBoxLayout;
    m_addStepButton = new QPushButton("+ 添加步骤");
    m_addStepButton->setObjectName("addStepBtn");
    QPushButton *delBtn = new QPushButton("- 删除选中");
    delBtn->setObjectName("delStepBtn");
    delBtn->setStyleSheet(kDarkStyle);
    btnRow->addWidget(m_addStepButton);
    btnRow->addWidget(delBtn);
    connect(m_addStepButton, &QPushButton::clicked, this, &FlowchartStepEditor::onAddStep);
    connect(delBtn, &QPushButton::clicked, this, [this]() {
        int row = m_stepListWidget->currentRow();
        if (row >= 0 && row < m_steps.size()) {
            m_steps.removeAt(row);
            // 重新编号所有step_id
            for (int i = 0; i < m_steps.size(); ++i) {
                QVariantMap s = m_steps[i].toMap();
                s["step_id"] = QString("s%1").arg(i);
                // 更新connections中的target引用
                QVariantList conns = s["connections"].toList();
                for (int j = 0; j < conns.size(); ++j) {
                    QVariantMap c = conns[j].toMap();
                    int oldTgt = c["target"].toString().mid(1).toInt();
                    if (oldTgt > row) c["target"] = QString("s%1").arg(oldTgt - 1);
                    conns[j] = c;
                }
                s["connections"] = conns;
                m_steps[i] = s;
            }
            rebuildStepList();
            saveCurrentStep();
            emit valueChanged();
        }
    });
    editLayout->addLayout(btnRow);

    m_tabWidget->addTab(editTab, "步骤编辑");

    // ==== 标签2: 全局样式 ====
    QScrollArea *styleScroll = new QScrollArea;
    styleScroll->setWidgetResizable(true);
    styleScroll->setStyleSheet(kDarkStyle);
    styleScroll->setWidget(createGlobalStylePanel());
    m_tabWidget->addTab(styleScroll, "全局样式");

    mainLayout->addWidget(m_tabWidget, 1);

    // ---- 预览区（动态高度）----
    m_previewLabel = new QLabel;
    m_previewLabel->setMinimumHeight(80);
    m_previewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("background-color:#1e1e1e;border:1px solid #4a4a4a;color:#888;");
    m_previewLabel->setText("预览");
    mainLayout->addWidget(m_previewLabel, 1);  // stretch=1 拉伸占满剩余空间

    // 默认全局样式
    m_globalStyle["colorScheme"] = "Default";
    m_globalStyle["borderColor"] = QVariantList{78, 201, 176};
    m_borderColor = QColor(78, 201, 176);
    m_globalStyle["fontSize"] = 12;
    m_globalStyle["canvasWidth"] = 1920;
    m_globalStyle["canvasHeight"] = 1080;
    m_globalStyle["spacing"] = 40;
    m_globalStyle["direction"] = "TopToBottom";
    m_globalStyle["connectorStyle"] = "Orthogonal";

    setStyleSheet(kDarkStyle);
}

// ---- 单个步骤控件组 ----

QWidget *FlowchartStepEditor::createStepWidget(int index, const QVariantMap &data)
{
    QString lt = data.value("logic_type", "过程/操作").toString();
    QGroupBox *group = new QGroupBox(QString("步骤%1 [%2]").arg(index + 1).arg(lt));
    group->setCheckable(false);
    group->setStyleSheet(kDarkStyle);

    QVBoxLayout *gLayout = new QVBoxLayout(group); gLayout->setSpacing(6); gLayout->setContentsMargins(8,8,8,8);

    // --- 名称 ---
    QHBoxLayout *nameRow = new QHBoxLayout;
    nameRow->addWidget(new QLabel("名称:"));
    QLineEdit *nameEdit = new QLineEdit(data.value("step_name", "").toString());
    nameEdit->setObjectName("step_name");
    nameEdit->setPlaceholderText("Step name...");
    connect(nameEdit, &QLineEdit::textChanged, this, &FlowchartStepEditor::onStepChanged);
    nameRow->addWidget(nameEdit);
    gLayout->addLayout(nameRow);

    // --- 逻辑类型（形状自动确定，无需手动选择）---
    QHBoxLayout *typeRow = new QHBoxLayout;
    typeRow->addWidget(new QLabel("逻辑类型:"));
    QComboBox *logicCombo = new QComboBox;
    logicCombo->setObjectName("logic_type");
    logicCombo->addItems({"开始","过程/操作","判断/分支","子流程","输入/输出","结束","等待/延迟","并行开始","并行结束"});
    QString curLogic = data.value("logic_type", "过程/操作").toString();
    logicCombo->setCurrentText(curLogic);
    typeRow->addWidget(logicCombo);
    connect(logicCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FlowchartStepEditor::onStepChanged);
    typeRow->addStretch();
    gLayout->addLayout(typeRow);

    // --- 连接到（多选其他步骤） ---
    QHBoxLayout *connRow = new QHBoxLayout;
    connRow->addWidget(new QLabel("连接到:"));
    QListWidget *connList = new QListWidget;
    connList->setObjectName("conn_list");
    connList->setSelectionMode(QAbstractItemView::MultiSelection);
    connList->setMaximumHeight(60);
    connList->setStyleSheet(kDarkStyle);
    connList->blockSignals(true);
    for (int i = 0; i < m_steps.size(); ++i) {
        if (i == index) continue;
        QVariantMap s = m_steps[i].toMap();
        QString name = s.value("step_name", QString("Step %1").arg(i + 1)).toString();
        QListWidgetItem *lwi = new QListWidgetItem(
            QString("[%1] %2").arg(i + 1).arg(name));
        lwi->setData(Qt::UserRole, i);  // 存储步骤索引
        // 恢复选中状态（target是"si"格式的step_id）
        QVariantList conns = data.value("connections").toList();
        for (const QVariant &c : conns) {
            QVariantMap cm = c.toMap();
            if (cm.value("target").toString() == QString("s%1").arg(i)) {
                lwi->setSelected(true);
                break;
            }
        }
        connList->addItem(lwi);
    }
    connList->blockSignals(false);
    connect(connList, &QListWidget::itemSelectionChanged, this, &FlowchartStepEditor::onStepChanged);
    connRow->addWidget(connList);
    gLayout->addLayout(connRow);

    // --- 判断分支目标（仅判断/分支类型显示）---
    if (lt == "Decision" || lt == "判断/分支") {
        for (int pass = 0; pass < 2; ++pass) {
            QString key = (pass == 0) ? "branch_true" : "branch_false";
            QString label = (pass == 0) ? "是→:" : "否→:";
            QHBoxLayout *row = new QHBoxLayout;
            row->addWidget(new QLabel(label));
            QComboBox *cb = new QComboBox;
            cb->setObjectName(key);
            cb->addItem("(无)", -1);
            for (int i = 0; i < m_steps.size(); ++i) {
                if (i == index) continue;
                QVariantMap s = m_steps[i].toMap();
                cb->addItem(QString("[%1] %2").arg(i+1).arg(s.value("step_name","").toString()), i);
            }
            int saved = data.value(key, -1).toInt();
            int si = cb->findData(saved);
            if (si >= 0) cb->setCurrentIndex(si);
            connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FlowchartStepEditor::onStepChanged);
            row->addWidget(cb, 1);
            gLayout->addLayout(row);
        }
    }

    // --- 删除按钮 ---
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    QPushButton *delBtn = new QPushButton("Delete Step");
    delBtn->setObjectName("delStepBtn");
    delBtn->setStyleSheet(kDarkStyle);
    connect(delBtn, &QPushButton::clicked, this, [this, index]() {
        onDeleteStep(index);
    });
    btnRow->addWidget(delBtn);
    gLayout->addLayout(btnRow);

    return group;
}

// ---- 全局样式面板 ----

QWidget *FlowchartStepEditor::createGlobalStylePanel()
{
    QGroupBox *group = new QGroupBox("Global Style");
    group->setStyleSheet(kDarkStyle);
    QGridLayout *grid = new QGridLayout(group);

    int row = 0;

    // 配色方案
    grid->addWidget(new QLabel("配色方案:"), row, 0);
    m_colorSchemeCombo = new QComboBox;
    m_colorSchemeCombo->addItems({"Default", "Pastel", "Bold", "Monochrome", "High Contrast"});
    m_colorSchemeCombo->setCurrentText(m_globalStyle.value("colorScheme", "Default").toString());
    connect(m_colorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_colorSchemeCombo, row++, 1);

    // 边框颜色
    grid->addWidget(new QLabel("边框颜色:"), row, 0);
    m_borderColorBtn = new QPushButton;
    m_borderColorBtn->setObjectName("colorBtn");
    m_borderColorBtn->setStyleSheet(
        QString("background-color: %1; min-width: 32px; min-height: 22px;"
                "border: 1px solid #4a4a4a; border-radius: 2px;")
            .arg(m_borderColor.name()));
    connect(m_borderColorBtn, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(m_borderColor, this, "选择边框颜色");
        if (c.isValid()) {
            m_borderColor = c;
            m_borderColorBtn->setStyleSheet(
                QString("background-color: %1; min-width: 32px; min-height: 22px;"
                        "border: 1px solid #4a4a4a; border-radius: 2px;")
                    .arg(c.name()));
            onGlobalStyleChanged();
        }
    });
    grid->addWidget(m_borderColorBtn, row++, 1);

    // 字体大小
    grid->addWidget(new QLabel("字号:"), row, 0);
    m_fontSizeSpin = new QSpinBox;
    m_fontSizeSpin->setRange(6, 48);
    m_fontSizeSpin->setValue(m_globalStyle.value("fontSize", 12).toInt());
    connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_fontSizeSpin, row++, 1);

    // 画布尺寸
    grid->addWidget(new QLabel("画布宽:"), row, 0);
    m_canvasWidthSpin = new QSpinBox;
    m_canvasWidthSpin->setRange(320, 8000);
    m_canvasWidthSpin->setSingleStep(100);
    m_canvasWidthSpin->setValue(m_globalStyle.value("canvasWidth", 1920).toInt());
    connect(m_canvasWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_canvasWidthSpin, row++, 1);

    grid->addWidget(new QLabel("画布高:"), row, 0);
    m_canvasHeightSpin = new QSpinBox;
    m_canvasHeightSpin->setRange(200, 8000);
    m_canvasHeightSpin->setSingleStep(100);
    m_canvasHeightSpin->setValue(m_globalStyle.value("canvasHeight", 1080).toInt());
    connect(m_canvasHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_canvasHeightSpin, row++, 1);

    // 间距
    grid->addWidget(new QLabel("间距:"), row, 0);
    m_spacingSpin = new QSpinBox;
    m_spacingSpin->setRange(10, 200);
    m_spacingSpin->setValue(m_globalStyle.value("spacing", 40).toInt());
    connect(m_spacingSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_spacingSpin, row++, 1);

    // 方向
    grid->addWidget(new QLabel("方向:"), row, 0);
    m_directionCombo = new QComboBox;
    m_directionCombo->addItems({"TopToBottom", "BottomToTop", "LeftToRight", "RightToLeft"});
    m_directionCombo->setCurrentText(m_globalStyle.value("direction", "TopToBottom").toString());
    connect(m_directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_directionCombo, row++, 1);

    // 连接线样式
    grid->addWidget(new QLabel("连线样式:"), row, 0);
    m_connectorStyleCombo = new QComboBox;
    m_connectorStyleCombo->addItems({"Orthogonal", "Straight", "Curved"});
    m_connectorStyleCombo->setCurrentText(m_globalStyle.value("connectorStyle", "Orthogonal").toString());
    connect(m_connectorStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FlowchartStepEditor::onGlobalStyleChanged);
    grid->addWidget(m_connectorStyleCombo, row++, 1);

    return group;
}

// ---- 构建列表 ----

void FlowchartStepEditor::rebuildStepList()
{
    m_stepListWidget->blockSignals(true);
    int oldRow = m_stepListWidget->currentRow();
    m_stepListWidget->clear();
    for (int i = 0; i < m_steps.size(); ++i) {
        QVariantMap s = m_steps[i].toMap();
        QString name = s.value("step_name", QString("Step %1").arg(i + 1)).toString();
        m_stepListWidget->addItem(
            QString("[%1] %2").arg(s.value("logic_type", "Process").toString(), name));
    }
    int sel = (oldRow >= 0 && oldRow < m_stepListWidget->count()) ? oldRow
            : (m_stepListWidget->count() > 0 ? 0 : -1);
    if (sel >= 0) {
        m_stepListWidget->setCurrentRow(sel);
    }
    m_stepListWidget->blockSignals(false);
    // 手动触发一次当前步骤显示（因为blockSignals阻止了信号）
    if (sel >= 0 && sel < m_steps.size())
        onCurrentStepChanged(sel);
    updatePreview();
}

// ---- 数据获取/设置 ----

void FlowchartStepEditor::setSteps(const QVariantList &steps)
{
    m_steps = steps;
    rebuildStepList();
}

QVariantList FlowchartStepEditor::steps() const
{
    return m_steps;
}

void FlowchartStepEditor::setGlobalStyle(const QVariantMap &style)
{
    m_globalStyle = style;

    // 同步到控件
    if (m_colorSchemeCombo)
        m_colorSchemeCombo->setCurrentText(m_globalStyle.value("colorScheme", "Default").toString());

    QVariantList bc = m_globalStyle.value("borderColor").toList();
    if (bc.size() >= 3) {
        m_borderColor = QColor(bc[0].toInt(), bc[1].toInt(), bc[2].toInt());
        if (m_borderColorBtn)
            m_borderColorBtn->setStyleSheet(
                QString("background-color: %1; min-width: 32px; min-height: 22px;"
                        "border: 1px solid #4a4a4a; border-radius: 2px;")
                    .arg(m_borderColor.name()));
    }

    if (m_fontSizeSpin)
        m_fontSizeSpin->setValue(m_globalStyle.value("fontSize", 12).toInt());
    if (m_canvasWidthSpin)
        m_canvasWidthSpin->setValue(m_globalStyle.value("canvasWidth", 1920).toInt());
    if (m_canvasHeightSpin)
        m_canvasHeightSpin->setValue(m_globalStyle.value("canvasHeight", 1080).toInt());
    if (m_spacingSpin)
        m_spacingSpin->setValue(m_globalStyle.value("spacing", 40).toInt());
    if (m_directionCombo)
        m_directionCombo->setCurrentText(m_globalStyle.value("direction", "TopToBottom").toString());
    if (m_connectorStyleCombo)
        m_connectorStyleCombo->setCurrentText(m_globalStyle.value("connectorStyle", "Orthogonal").toString());

    updatePreview();
}

QVariantMap FlowchartStepEditor::globalStyle() const
{
    return m_globalStyle;
}

// ---- Slots ----

void FlowchartStepEditor::onAddStep()
{
    QVariantMap step;
    step["step_id"] = QString("s%1").arg(m_steps.size());
    step["step_name"] = QString("Step %1").arg(m_steps.size() + 1);
    step["logic_type"] = "过程/操作";
    step["connections"] = QVariantList();
    step["branch_true"] = "";
    step["branch_false"] = "";

    m_steps.append(step);
    rebuildStepList();
    emit valueChanged();
}

void FlowchartStepEditor::onDeleteStep(int index)
{
    if (index >= 0 && index < m_steps.size()) {
        m_steps.removeAt(index);
        rebuildStepList();
        emit valueChanged();
    }
}

void FlowchartStepEditor::saveCurrentStep()
{
    // 保存当前可见的步骤详情回 m_steps
    int idx = m_stepListWidget->currentRow();
    if (idx < 0 || idx >= m_steps.size()) return;
    if (!m_stepDetailWidget || m_stepDetailWidget->count() == 0) return;

    QWidget* w = m_stepDetailWidget->currentWidget();
    if (!w) return;
    QVariantMap data = m_steps[idx].toMap();

    auto edits = w->findChildren<QLineEdit*>();
    for (auto* ed : edits) {
        if (ed->objectName() == "step_name") data["step_name"] = ed->text();
    }
    auto combos = w->findChildren<QComboBox*>();
    for (auto* cb : combos) {
        if (cb->objectName() == "logic_type") data["logic_type"] = cb->currentText();
        else if (cb->objectName() == "branch_true") data["branch_true"] = cb->currentData().toInt();
        else if (cb->objectName() == "branch_false") data["branch_false"] = cb->currentData().toInt();
    }
    auto lists = w->findChildren<QListWidget*>();
    for (auto* lw : lists) {
        if (lw->objectName() == "conn_list") {
            QVariantList conns;
            for (int i = 0; i < lw->count(); ++i) {
                if (lw->item(i)->isSelected()) {
                    int targetIdx = lw->item(i)->data(Qt::UserRole).toInt();
                    QVariantMap c;
                    c["target"] = QString("s%1").arg(targetIdx); // step_id格式
                    c["label"] = "";
                    conns << c;
                }
            }
            data["connections"] = conns;
        }
    }
    // 确保有step_id
    if (!data.contains("step_id") || data["step_id"].toString().isEmpty())
        data["step_id"] = QString("s%1").arg(idx);
    m_steps[idx] = data;
    rebuildStepList();
}

void FlowchartStepEditor::onStepChanged()
{
    saveCurrentStep();
}

void FlowchartStepEditor::onCurrentStepChanged(int row)
{
    if (row < 0 || row >= m_steps.size()) return;
    while (m_stepDetailWidget->count() > 0) {
        QWidget* w = m_stepDetailWidget->widget(0);
        m_stepDetailWidget->removeWidget(w);
        w->deleteLater();
    }
    QVariantMap data = m_steps[row].toMap();
    // 确保step_id存在
    if (!data.contains("step_id") || data["step_id"].toString().isEmpty())
        data["step_id"] = QString("s%1").arg(row);
    if (!data.contains("connections"))
        data["connections"] = QVariantList();
    QWidget* detail = createStepWidget(row, data);
    m_stepDetailWidget->addWidget(detail);
    m_stepDetailWidget->setCurrentIndex(0);
}
void FlowchartStepEditor::onStepOrderChanged()
{
    // 从 QListWidget 的顺序重排 m_steps
    QVariantList newOrder;
    for (int i = 0; i < m_stepListWidget->count(); ++i) {
        QListWidgetItem *lwi = m_stepListWidget->item(i);
        int oldIndex = lwi->data(Qt::UserRole).toInt();
        if (oldIndex >= 0 && oldIndex < m_steps.size())
            newOrder.append(m_steps[oldIndex]);
    }
    if (newOrder.size() == m_steps.size()) {
        m_steps = newOrder;
        rebuildStepList();
        emit valueChanged();
    }
}

void FlowchartStepEditor::onGlobalStyleChanged()
{
    if (m_colorSchemeCombo)
        m_globalStyle["colorScheme"] = m_colorSchemeCombo->currentText();

    QVariantList bc;
    bc << m_borderColor.red() << m_borderColor.green() << m_borderColor.blue();
    m_globalStyle["borderColor"] = bc;

    if (m_fontSizeSpin)
        m_globalStyle["fontSize"] = m_fontSizeSpin->value();
    if (m_canvasWidthSpin)
        m_globalStyle["canvasWidth"] = m_canvasWidthSpin->value();
    if (m_canvasHeightSpin)
        m_globalStyle["canvasHeight"] = m_canvasHeightSpin->value();
    if (m_spacingSpin)
        m_globalStyle["spacing"] = m_spacingSpin->value();
    if (m_directionCombo)
        m_globalStyle["direction"] = m_directionCombo->currentText();
    if (m_connectorStyleCombo)
        m_globalStyle["connectorStyle"] = m_connectorStyleCombo->currentText();

    updatePreview();
    emit valueChanged();
}

void FlowchartStepEditor::onTabChanged(int index)
{
    Q_UNUSED(index);
    // 切换标签页时可以做额外处理
}

void FlowchartStepEditor::updatePreview()
{
    if (!m_previewLabel)
        return;

    int pw = m_previewLabel->width();
    int ph = m_previewLabel->height();
    if (pw <= 0) pw = 300;
    if (ph <= 0) ph = 200;

    QPixmap pix(pw, ph);
    pix.fill(QColor(0x1e, 0x1e, 0x1e));

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_steps.isEmpty()) {
        painter.setPen(QColor(0x6a, 0x6a, 0x6a));
        painter.drawText(pix.rect(), Qt::AlignCenter, "(No steps)");
        m_previewLabel->setPixmap(pix);
        return;
    }

    QString direction = m_globalStyle.value("direction", "TopToBottom").toString();
    bool vertical = (direction == "TopToBottom" || direction == "BottomToTop");
    int spacing = m_globalStyle.value("spacing", 40).toInt();

    int totalSteps = m_steps.size();
    int stepW = vertical ? qMin(80, (pw - 20) / 2) : qMin(80, (pw - spacing * (totalSteps - 1) - 20) / totalSteps);
    int stepH = vertical ? qMin(40, (ph - spacing * (totalSteps - 1) - 20) / totalSteps) : qMin(40, (ph - 20) / 2);

    int startX = 10;
    int startY = 10;
    if (vertical)
        startX = (pw - stepW) / 2;
    else
        startY = (ph - stepH) / 2;

    // 形状映射（简化为矩形/菱形/圆角矩形）
    auto drawShape = [&](QPainter &p, const QRect &r, const QString &shape, const QString &logicType) {
        QColor fill(0x2a, 0x2a, 0x2a);
        // 不同逻辑类型用不同微调色调
        if (logicType == "Start" || logicType == "End")
            fill = QColor(0x2a, 0x4a, 0x2a);
        else if (logicType == "Decision")
            fill = QColor(0x4a, 0x2a, 0x2a);
        else if (logicType == "Subprocess")
            fill = QColor(0x2a, 0x2a, 0x4a);
        else if (logicType == "Wait")
            fill = QColor(0x4a, 0x4a, 0x2a);

        p.setBrush(fill);
        p.setPen(QPen(m_borderColor, 1));

        if (shape == "Diamond") {
            QPolygon diamond;
            diamond << QPoint(r.center().x(), r.top())
                    << QPoint(r.right(), r.center().y())
                    << QPoint(r.center().x(), r.bottom())
                    << QPoint(r.left(), r.center().y());
            p.drawPolygon(diamond);
        } else if (shape == "Ellipse") {
            p.drawEllipse(r);
        } else if (shape == "RoundedRect") {
            p.drawRoundedRect(r, 6, 6);
        } else {
            p.drawRect(r);
        }

        // 文字
        p.setPen(QColor(0xe0, 0xe0, 0xe0));
        QFont f = p.font();
        f.setPixelSize(qMax(6, qMin(stepH / 3, 10)));
        p.setFont(f);
        QVariantMap s = m_steps.isEmpty() ? QVariantMap() : m_steps[0].toMap();
        p.drawText(r, Qt::AlignCenter, s.value("step_name", "?").toString().left(8));
    };

    // 绘制步骤和连线
    QVector<QPoint> centers;
    for (int i = 0; i < totalSteps; ++i) {
        int x, y;
        if (vertical) {
            x = startX;
            y = startY + i * (stepH + spacing);
        } else {
            x = startX + i * (stepW + spacing);
            y = startY;
        }

        if (direction == "BottomToTop" && vertical)
            y = ph - 10 - stepH - i * (stepH + spacing);
        if (direction == "RightToLeft" && !vertical)
            x = pw - 10 - stepW - i * (stepW + spacing);

        QRect stepRect(x, y, stepW, stepH);
        QVariantMap s = m_steps[i].toMap();
        drawShape(painter, stepRect, s.value("shape", "Rect").toString(),
                  s.value("logic_type", "Process").toString());
        centers.append(stepRect.center());
    }

    // 连线
    painter.setPen(QPen(QColor(0x6a, 0x6a, 0x6a), 1, Qt::DotLine));
    for (int i = 0; i < centers.size() - 1; ++i) {
        painter.drawLine(
            vertical ? QPoint(centers[i].x(), centers[i].y() + stepH / 2)
                     : QPoint(centers[i].x() + stepW / 2, centers[i].y()),
            vertical ? QPoint(centers[i + 1].x(), centers[i + 1].y() - stepH / 2)
                     : QPoint(centers[i + 1].x() - stepW / 2, centers[i + 1].y()));
    }

    painter.end();
    m_previewLabel->setPixmap(pix);
}
