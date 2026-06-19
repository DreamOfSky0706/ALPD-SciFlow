// gui/PropertyPanel.cpp
#include "PropertyPanel.h"
#include "../core/WorkflowGraph.h"
#include "../core/NodeBase.h"
#include "CurveEditor.h"
#include "GridEditor.h"
#include "GradientEditor.h"
#include "AnnotationListEditor.h"
#include "FlowchartStepEditor.h"
#include "TableEditor.h"
#include "LegendListEditor.h"
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QColorDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>

PropertyPanel::PropertyPanel(WorkflowGraph* graph, QWidget* parent)
    : QWidget(parent)
    , m_graph(graph)
{
    setMinimumWidth(260);
    // 不设最大宽度，让QSplitter和内容自适应

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题区域
    auto* titleWidget = new QWidget();
    titleWidget->setStyleSheet("background-color: #333333; padding: 8px;");
    auto* titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setSpacing(2);

    m_titleLabel = new QLabel("属性面板");
    m_titleLabel->setStyleSheet("color: #e0e0e0; font-size: 14px; font-weight: bold;");
    m_typeLabel = new QLabel("");
    m_typeLabel->setStyleSheet("color: #a0a0a0; font-size: 11px;");

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_typeLabel);
    mainLayout->addWidget(titleWidget);

    // 滚动区域
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2b2b2b; } QScrollArea > QWidget > QWidget { background-color: #2b2b2b; }");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background-color: #2b2b2b;");
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(12, 8, 12, 8);
    m_contentLayout->setSpacing(6);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea, 1);

    setStyleSheet("background-color: #2b2b2b;");
    // 强制创建原生窗口句柄，子控件共享此句柄避免Windows下每个控件弹窗
    winId();
    m_contentWidget->winId();
    // 始终可见，避免visibility切换导致闪烁
    setVisible(true);
}

void PropertyPanel::showNode(const QString& nodeId)
{
    m_currentNodeId = nodeId;
    m_currentNode = m_graph->nodeById(nodeId);
    if (!m_currentNode) {
        clearNode();
        return;
    }

    rebuildUI();
    m_expanded = true;

    // 如果参数简单（≤4个参数且无复杂控件），可以考虑悬浮面板
    // 但属性面板仍然显示
}

void PropertyPanel::clearNode()
{
    m_currentNodeId.clear();
    m_currentNode = nullptr;
    m_expanded = false;
    rebuildUI();
    // 不清空时显示提示
    m_titleLabel->setText("属性面板");
    m_typeLabel->setText("未选中节点");
}

bool PropertyPanel::shouldExpand() const
{
    if (!m_currentNode) return false;
    const auto& params = m_currentNode->paramDefinitions();
    // 参数超过4个或包含复杂编辑器
    if (params.size() > 4) return true;
    for (const auto& p : params) {
        switch (p.type) {
            case ParamType::CurveEditor:
            case ParamType::TableEditor:
            case ParamType::GridEditor:
            case ParamType::GradientEditor:
            case ParamType::AnnotationList:
            case ParamType::FlowchartSteps:
            case ParamType::KernelMatrix:
            case ParamType::PointList:
            case ParamType::MultiLineString:
            case ParamType::LegendList:
            case ParamType::LineStyleList:
                return true;
            default: break;
        }
    }
    return false;
}

void PropertyPanel::rebuildUI()
{
    // 暂停重绘
    m_contentWidget->setUpdatesEnabled(false);

    // 清除旧控件
    m_paramControls.clear();
    m_paramLabels.clear();

    // 清除布局中的所有子widget
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    if (!m_currentNode) {
        m_titleLabel->setText("属性面板");
        m_typeLabel->setText("");
        m_contentLayout->addStretch();
        return;
    }

    m_titleLabel->setText(m_currentNode->displayName());
    m_typeLabel->setText(QString("(%1)").arg(m_currentNode->typeName()));

    // 为每个参数创建控件（所有子控件显式指定m_contentWidget为父，防Windows原生窗口）
    const auto& params = m_currentNode->paramDefinitions();
    for (const auto& paramDef : params) {
        QVariant currentVal = m_currentNode->param(paramDef.name);

        auto* row = new QWidget(m_contentWidget);
        auto* rowLayout = new QFormLayout(row);
        rowLayout->setContentsMargins(0, 2, 0, 2);
        rowLayout->setSpacing(4);

        auto* label = new QLabel(paramDef.label);
        label->setStyleSheet("color: #a0a0a0; font-size: 11px; min-width: 80px;");
        m_paramLabels[paramDef.name] = label;

        QWidget* control = createControl(paramDef, currentVal);
        if (control) {
            control->setParent(row); // 强制设父控件防Windows弹出原生窗口
            m_paramControls[paramDef.name] = control;
            rowLayout->addRow(label, control);
            row->setVisible(paramDef.visible);
        }

        m_contentLayout->addWidget(row);
    }

    m_contentLayout->addStretch();
    updateParamVisibility();
    m_contentWidget->setUpdatesEnabled(true);
}

QWidget* PropertyPanel::createControl(const ParamDefinition& paramDef,
                                       const QVariant& currentValue)
{
    QVariantMap c = paramDef.constraints;
    const QString& name = paramDef.name;

    switch (paramDef.type) {

    case ParamType::Int: {
        auto* spin = new QSpinBox();
        spin->setMinimum(c.value("min", -1000000).toInt());
        spin->setMaximum(c.value("max", 1000000).toInt());
        spin->setValue(currentValue.toInt());
        spin->setStyleSheet("QSpinBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 2px 6px; }");
        spin->setProperty("paramName", name);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &PropertyPanel::onParamControlChanged);
        return spin;
    }

    case ParamType::Double: {
        auto* spin = new QDoubleSpinBox();
        spin->setMinimum(c.value("min", -1000000.0).toDouble());
        spin->setMaximum(c.value("max", 1000000.0).toDouble());
        spin->setDecimals(c.value("decimals", 2).toInt());
        double step = c.value("step", 0.1).toDouble();
        spin->setSingleStep(step);
        spin->setValue(currentValue.toDouble());
        spin->setStyleSheet("QDoubleSpinBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 2px 6px; }");
        spin->setProperty("paramName", name);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &PropertyPanel::onParamControlChanged);
        return spin;
    }

    case ParamType::IntSlider: {
        auto* w = new QWidget();
        auto* lay = new QHBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);

        auto* slider = new QSlider(Qt::Horizontal);
        slider->setMinimum(c.value("min", 0).toInt());
        slider->setMaximum(c.value("max", 100).toInt());
        slider->setValue(currentValue.toInt());
        slider->setStyleSheet(
            "QSlider::groove:horizontal { background: #4a4a4a; height: 4px; border-radius: 2px; }"
            "QSlider::handle:horizontal { background: #4EC9B0; width: 12px; margin: -5px 0; border-radius: 6px; }");

        auto* spin = new QSpinBox();
        spin->setMinimum(slider->minimum());
        spin->setMaximum(slider->maximum());
        spin->setValue(currentValue.toInt());
        spin->setFixedWidth(60);
        spin->setStyleSheet("QSpinBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 2px; }");

        connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), slider, &QSlider::setValue);

        slider->setProperty("paramName", name);
        spin->setProperty("paramName", name);
        connect(slider, &QSlider::valueChanged, this, &PropertyPanel::onParamControlChanged);

        lay->addWidget(slider, 1);
        lay->addWidget(spin);
        return w;
    }

    case ParamType::DoubleSlider: {
        auto* w = new QWidget();
        auto* lay = new QHBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);

        int intMin = 0, intMax = 1000;
        double dMin = c.value("min", 0.0).toDouble();
        double dMax = c.value("max", 1.0).toDouble();

        auto* slider = new QSlider(Qt::Horizontal);
        slider->setMinimum(intMin);
        slider->setMaximum(intMax);
        slider->setValue(qRound((currentValue.toDouble() - dMin) / (dMax - dMin) * intMax));
        slider->setStyleSheet(
            "QSlider::groove:horizontal { background: #4a4a4a; height: 4px; border-radius: 2px; }"
            "QSlider::handle:horizontal { background: #4EC9B0; width: 12px; margin: -5px 0; border-radius: 6px; }");

        auto* spin = new QDoubleSpinBox();
        spin->setMinimum(dMin);
        spin->setMaximum(dMax);
        spin->setDecimals(c.value("decimals", 2).toInt());
        spin->setValue(currentValue.toDouble());
        spin->setFixedWidth(60);
        spin->setStyleSheet("QDoubleSpinBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 2px; }");

        connect(slider, &QSlider::valueChanged, spin,
                [=](int v) { spin->setValue(dMin + (dMax - dMin) * v / intMax); });
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), slider,
                [=](double v) { slider->setValue(qRound((v - dMin) / (dMax - dMin) * intMax)); });

        slider->setProperty("paramName", name);
        spin->setProperty("paramName", name);
        connect(slider, &QSlider::valueChanged, this, &PropertyPanel::onParamControlChanged);

        lay->addWidget(slider, 1);
        lay->addWidget(spin);
        return w;
    }

    case ParamType::String: {
        auto* edit = new QLineEdit();
        edit->setText(currentValue.toString());
        edit->setStyleSheet("QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }");
        edit->setProperty("paramName", name);
        connect(edit, &QLineEdit::editingFinished, this, &PropertyPanel::onParamControlChanged);
        return edit;
    }

    case ParamType::MultiLineString: {
        auto* edit = new QTextEdit();
        edit->setPlainText(currentValue.toString());
        edit->setMaximumHeight(100);
        edit->setStyleSheet("QTextEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; }");
        edit->setProperty("paramName", name);
        connect(edit, &QTextEdit::textChanged, this, [this, edit]() {
            Q_UNUSED(edit)
            // 延迟处理避免频繁更新
        });
        return edit;
    }

    case ParamType::FilePath: {
        auto* w = new QWidget();
        auto* lay = new QHBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);

        auto* edit = new QLineEdit();
        edit->setText(currentValue.toString());
        edit->setStyleSheet("QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }");
        edit->setProperty("paramName", name);
        connect(edit, &QLineEdit::editingFinished, this, &PropertyPanel::onParamControlChanged);

        auto* btn = new QPushButton("浏览");
        btn->setFixedWidth(50);
        btn->setStyleSheet("QPushButton { background-color: #404040; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 8px; } QPushButton:hover { background-color: #505050; }");
        connect(btn, &QPushButton::clicked, this, [this, edit, name]() {
            QString path = QFileDialog::getOpenFileName(this, "选择文件",
                edit->text().isEmpty() ? QDir::homePath() : QFileInfo(edit->text()).absolutePath());
            if (!path.isEmpty()) {
                edit->setText(path);
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, name, path);
            }
        });

        lay->addWidget(edit, 1);
        lay->addWidget(btn);
        return w;
    }

    case ParamType::SaveFilePath: {
        auto* w = new QWidget();
        auto* lay = new QHBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);

        auto* edit = new QLineEdit();
        edit->setText(currentValue.toString());
        edit->setStyleSheet("QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }");
        edit->setProperty("paramName", name);
        connect(edit, &QLineEdit::editingFinished, this, &PropertyPanel::onParamControlChanged);

        auto* btn = new QPushButton("浏览");
        btn->setFixedWidth(50);
        btn->setStyleSheet("QPushButton { background-color: #404040; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 8px; } QPushButton:hover { background-color: #505050; }");
        connect(btn, &QPushButton::clicked, this, [this, edit, name]() {
            QString path = QFileDialog::getSaveFileName(this, "保存文件",
                edit->text().isEmpty() ? QDir::homePath() : edit->text());
            if (!path.isEmpty()) {
                edit->setText(path);
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, name, path);
            }
        });

        lay->addWidget(edit, 1);
        lay->addWidget(btn);
        return w;
    }

    case ParamType::DirPath: {
        auto* w = new QWidget();
        auto* lay = new QHBoxLayout(w);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(4);

        auto* edit = new QLineEdit();
        edit->setText(currentValue.toString());
        edit->setStyleSheet("QLineEdit { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }");
        edit->setProperty("paramName", name);
        connect(edit, &QLineEdit::editingFinished, this, &PropertyPanel::onParamControlChanged);

        auto* btn = new QPushButton("浏览");
        btn->setFixedWidth(50);
        btn->setStyleSheet("QPushButton { background-color: #404040; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 8px; } QPushButton:hover { background-color: #505050; }");
        connect(btn, &QPushButton::clicked, this, [this, edit, name]() {
            QString path = QFileDialog::getExistingDirectory(this, "选择目录");
            if (!path.isEmpty()) {
                edit->setText(path);
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, name, path);
            }
        });

        lay->addWidget(edit, 1);
        lay->addWidget(btn);
        return w;
    }

    case ParamType::Combo: {
        auto* combo = new QComboBox();
        QStringList options = c.value("options").toStringList();
        combo->addItems(options);
        int idx = options.indexOf(currentValue.toString());
        if (idx >= 0) combo->setCurrentIndex(idx);
        combo->setStyleSheet(
            "QComboBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }"
            "QComboBox::drop-down { background-color: #404040; border: none; }"
            "QComboBox QAbstractItemView { background-color: #3a3a3a; color: #e0e0e0; selection-background-color: #4EC9B0; }");
        combo->setProperty("paramName", name);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &PropertyPanel::onParamControlChanged);
        return combo;
    }

    case ParamType::Bool: {
        auto* check = new QCheckBox();
        check->setChecked(currentValue.toBool());
        check->setStyleSheet("QCheckBox { color: #e0e0e0; } QCheckBox::indicator { background-color: #3a3a3a; border: 1px solid #4a4a4a; width: 16px; height: 16px; } QCheckBox::indicator:checked { background-color: #4EC9B0; border-color: #4EC9B0; }");
        check->setProperty("paramName", name);
        connect(check, &QCheckBox::toggled, this, &PropertyPanel::onParamControlChanged);
        return check;
    }

    case ParamType::Color: {
        auto* btn = new QPushButton();
        QColor clr;
        QVariantList arr = currentValue.toList();
        if (arr.size() >= 4)
            clr = QColor(arr[0].toInt(), arr[1].toInt(), arr[2].toInt(), arr[3].toInt());
        else if (arr.size() >= 3)
            clr = QColor(arr[0].toInt(), arr[1].toInt(), arr[2].toInt());
        else
            clr = currentValue.value<QColor>();

        btn->setStyleSheet(QString("QPushButton { background-color: %1; border: 1px solid #4a4a4a; min-width: 30px; min-height: 24px; }").arg(clr.name(QColor::HexArgb)));
        btn->setProperty("paramName", name);
        btn->setProperty("currentColor", clr);
        connect(btn, &QPushButton::clicked, this, [this, btn, name]() {
            QColor newColor = QColorDialog::getColor(
                btn->property("currentColor").value<QColor>(),
                this, "选择颜色", QColorDialog::ShowAlphaChannel);
            if (newColor.isValid()) {
                btn->setProperty("currentColor", newColor);
                btn->setStyleSheet(QString("QPushButton { background-color: %1; border: 1px solid #4a4a4a; min-width: 30px; min-height: 24px; }").arg(newColor.name(QColor::HexArgb)));
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, name,
                        QVariantList({newColor.red(), newColor.green(), newColor.blue(), newColor.alpha()}));
            }
        });
        return btn;
    }

    case ParamType::Font: {
        auto* combo = new QFontComboBox();
        combo->setCurrentFont(QFont(currentValue.toString()));
        combo->setStyleSheet("QFontComboBox { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 4px 6px; }");
        combo->setProperty("paramName", name);
        connect(combo, &QFontComboBox::currentFontChanged,
                this, &PropertyPanel::onParamControlChanged);
        return combo;
    }

    case ParamType::CurveEditor: {
        auto* editor = new CurveEditor();
        QVariantList pts = currentValue.toList();
        QVector<QPointF> points;
        for (const auto& v : pts) {
            QVariantList pair = v.toList();
            if (pair.size() == 2)
                points.append(QPointF(pair[0].toDouble(), pair[1].toDouble()));
        }
        editor->setControlPoints(points);
        editor->setProperty("paramName", name);
        connect(editor, &CurveEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::GridEditor: {
        auto* editor = new GridEditor();
        editor->setGridConfig(currentValue.toMap());
        editor->setProperty("paramName", name);
        connect(editor, &GridEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::GradientEditor: {
        auto* editor = new GradientEditor();
        editor->setColorStops(currentValue.toList());
        editor->setProperty("paramName", name);
        connect(editor, &GradientEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::AnnotationList: {
        auto* editor = new AnnotationListEditor();
        editor->setAnnotations(currentValue.toList());
        editor->setProperty("paramName", name);
        connect(editor, &AnnotationListEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::FlowchartSteps: {
        auto* editor = new FlowchartStepEditor();
        editor->setSteps(currentValue.toList());
        editor->setProperty("paramName", name);
        connect(editor, &FlowchartStepEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::TableEditor: {
        auto* editor = new TableEditor();
        editor->setTableData(currentValue.toMap());
        editor->setProperty("paramName", name);
        connect(editor, &TableEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::LegendList: {
        auto* editor = new LegendListEditor();
        editor->setItems(currentValue.toList());
        editor->setProperty("paramName", name);
        connect(editor, &LegendListEditor::valueChanged, this, &PropertyPanel::onParamControlChanged);
        return editor;
    }
    case ParamType::KernelMatrix:
    case ParamType::PointList:
    case ParamType::MultiCombo:
    case ParamType::LineStyleList:
    default: {
        auto* label = new QLabel(QString("(在悬浮面板编辑)"));
        label->setStyleSheet("color: #808080; font-size: 11px; font-style: italic;");
        return label;
    }

    }
}

void PropertyPanel::onParamControlChanged()
{
    if (!m_currentNode || m_currentNodeId.isEmpty()) return;
    // 二次校验节点是否仍存在于图中
    if (!m_graph->nodeById(m_currentNodeId)) { clearNode(); return; }

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (!sender) return;

    QString paramName = sender->property("paramName").toString();
    if (paramName.isEmpty()) return;

    QVariant newValue;

    if (auto* spin = qobject_cast<QSpinBox*>(sender)) {
        newValue = spin->value();
    } else if (auto* dspin = qobject_cast<QDoubleSpinBox*>(sender)) {
        newValue = dspin->value();
    } else if (auto* slider = qobject_cast<QSlider*>(sender)) {
        newValue = slider->value();
    } else if (auto* edit = qobject_cast<QLineEdit*>(sender)) {
        newValue = edit->text();
    } else if (auto* combo = qobject_cast<QComboBox*>(sender)) {
        newValue = combo->currentText();
    } else if (auto* check = qobject_cast<QCheckBox*>(sender)) {
        newValue = check->isChecked();
    } else if (auto* btn = qobject_cast<QPushButton*>(sender)) {
        QColor clr = btn->property("currentColor").value<QColor>();
        newValue = QVariantList({clr.red(), clr.green(), clr.blue(), clr.alpha()});
    } else if (auto* fcombo = qobject_cast<QFontComboBox*>(sender)) {
        newValue = fcombo->currentFont().family();
    } else if (auto* curve = qobject_cast<CurveEditor*>(sender)) {
        QVariantList pts; for (auto& p : curve->controlPoints()) pts << QVariantList({p.x(), p.y()});
        newValue = pts;
    } else if (auto* grid = qobject_cast<GridEditor*>(sender)) {
        newValue = grid->gridConfig();
    } else if (auto* grad = qobject_cast<GradientEditor*>(sender)) {
        newValue = grad->colorStops();
    } else if (auto* ann = qobject_cast<AnnotationListEditor*>(sender)) {
        newValue = ann->annotations();
    } else if (auto* flow = qobject_cast<FlowchartStepEditor*>(sender)) {
        newValue = flow->steps();
    } else if (auto* tbl = qobject_cast<TableEditor*>(sender)) {
        newValue = tbl->tableData();
    } else if (auto* leg = qobject_cast<LegendListEditor*>(sender)) {
        newValue = leg->items();
    }

    if (!newValue.isNull() && !m_currentNodeId.isEmpty()) {
        m_graph->changeParam(m_currentNodeId, paramName, newValue);
        updateParamVisibility();
        // 联动GridEditor：行列数变化时同步网格尺寸和grid_config
        if ((paramName == "rows" || paramName == "cols") && m_paramControls.contains("grid_config")) {
            if (auto* grid = qobject_cast<GridEditor*>(m_paramControls["grid_config"])) {
                int r = m_currentNode->param("rows").toInt();
                int c = m_currentNode->param("cols").toInt();
                grid->setGridSize(qBound(1,r,4), qBound(1,c,4));
                // 同时更新grid_config中的rows/cols字段
                QVariantMap gc = grid->gridConfig();
                gc["rows"] = r; gc["cols"] = c;
                m_graph->changeParam(m_currentNodeId, "grid_config", gc, false);
            }
        }
        emit paramChanged(m_currentNodeId, paramName, newValue);
    }
}

void PropertyPanel::updateParamVisibility()
{
    if (!m_currentNode) return;

    const auto& params = m_currentNode->paramDefinitions();
    for (const auto& paramDef : params) {
        bool visible = paramDef.visible;

        // 简单条件处理：如果visibleCondition包含"mode=="
        if (!paramDef.visibleCondition.isEmpty()) {
            QString cond = paramDef.visibleCondition;
            // 非常简单的条件解析：仅支持 "paramName==value" 格式
            if (cond.contains("==")) {
                QStringList parts = cond.split("==");
                if (parts.size() == 2) {
                    QString condParam = parts[0].trimmed();
                    QString condValue = parts[1].trimmed();
                    QVariant actual = m_currentNode->param(condParam);
                    visible = (actual.toString() == condValue);
                }
            }
        }

        if (m_paramControls.contains(paramDef.name)) {
            m_paramControls[paramDef.name]->parentWidget()->setVisible(visible);
        }
        if (m_paramLabels.contains(paramDef.name)) {
            m_paramLabels[paramDef.name]->setVisible(visible);
        }
    }
}
