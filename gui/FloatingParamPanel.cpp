// gui/FloatingParamPanel.cpp
#include "FloatingParamPanel.h"
#include "../core/WorkflowGraph.h"
#include "../core/NodeBase.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QFontComboBox>
#include <QFrame>
#include <QLineEdit>
#include <QCheckBox>
#include <QEvent>
#include <QApplication>
#include <QScreen>

FloatingParamPanel::FloatingParamPanel(WorkflowGraph* graph, QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint)
    , m_graph(graph)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background-color: #383838; border: 2px solid #4EC9B0; border-radius: 8px;");
    setMaximumWidth(MAX_WIDTH);
}

void FloatingParamPanel::showForNode(const QString& nodeId, const QPoint& screenPos)
{
    m_currentNodeId = nodeId;
    m_currentNode = m_graph->nodeById(nodeId);
    if (!m_currentNode) return;

    rebuildUI();

    // 计算位置，确保不超出屏幕
    QPoint pos = screenPos + QPoint(10, 10);
    if (auto* screen = QApplication::screenAt(screenPos)) {
        QRect screenGeom = screen->availableGeometry();
        if (pos.x() + width() > screenGeom.right())
            pos.setX(screenGeom.right() - width());
        if (pos.y() + height() > screenGeom.bottom())
            pos.setY(screenGeom.bottom() - height());
    }

    move(pos);
    show();
}

void FloatingParamPanel::hide()
{
    m_currentNodeId.clear();
    m_currentNode = nullptr;
    QWidget::hide();
}

bool FloatingParamPanel::event(QEvent* event)
{
    // 失去焦点时关闭
    if (event->type() == QEvent::WindowDeactivate) {
        hide();
        return true;
    }
    return QWidget::event(event);
}

void FloatingParamPanel::rebuildUI()
{
    // 清除旧布局
    if (layout()) {
        QLayoutItem* item;
        while ((item = layout()->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }
        delete layout();
    }
    m_paramControls.clear();

    if (!m_currentNode) return;

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(4);

    // 标题
    auto* titleLabel = new QLabel(m_currentNode->displayName());
    titleLabel->setStyleSheet("color: #e0e0e0; font-size: 12px; font-weight: bold; border: none;");
    mainLayout->addWidget(titleLabel);

    // 分隔线
    auto* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #4a4a4a; border: none;");
    sep->setFixedHeight(1);
    mainLayout->addWidget(sep);

    const auto& params = m_currentNode->paramDefinitions();
    for (const auto& paramDef : params) {
        if (!paramDef.visible) continue;

        QVariant val = m_currentNode->param(paramDef.name);
        QVariantMap c = paramDef.constraints;

        auto* row = new QWidget();
        row->setStyleSheet("background: transparent; border: none;");
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 2, 0, 2);
        rowLayout->setSpacing(6);

        auto* label = new QLabel(paramDef.label);
        label->setStyleSheet("color: #a0a0a0; font-size: 11px; border: none;");
        label->setFixedWidth(70);

        QWidget* control = nullptr;
        QString style = "background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #4a4a4a; padding: 2px 4px;";

        switch (paramDef.type) {
        case ParamType::Int: {
            auto* spin = new QSpinBox();
            spin->setMinimum(c.value("min", -1000000).toInt());
            spin->setMaximum(c.value("max", 1000000).toInt());
            spin->setValue(val.toInt());
            spin->setStyleSheet(style);
            spin->setProperty("paramName", paramDef.name);
            connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, spin]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        spin->property("paramName").toString(), spin->value());
                }
            });
            control = spin;
            break;
        }
        case ParamType::Double: {
            auto* spin = new QDoubleSpinBox();
            spin->setMinimum(c.value("min", -1000000.0).toDouble());
            spin->setMaximum(c.value("max", 1000000.0).toDouble());
            spin->setValue(val.toDouble());
            spin->setStyleSheet(style);
            spin->setProperty("paramName", paramDef.name);
            connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, spin]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        spin->property("paramName").toString(), spin->value());
                }
            });
            control = spin;
            break;
        }
        case ParamType::String: {
            auto* edit = new QLineEdit(val.toString());
            edit->setStyleSheet(style);
            edit->setProperty("paramName", paramDef.name);
            connect(edit, &QLineEdit::editingFinished, this, [this, edit]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        edit->property("paramName").toString(), edit->text());
                }
            });
            control = edit;
            break;
        }
        case ParamType::Combo: {
            auto* combo = new QComboBox();
            combo->addItems(c.value("options").toStringList());
            int idx = c.value("options").toStringList().indexOf(val.toString());
            if (idx >= 0) combo->setCurrentIndex(idx);
            combo->setStyleSheet(style);
            combo->setProperty("paramName", paramDef.name);
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        combo->property("paramName").toString(), combo->currentText());
                }
            });
            control = combo;
            break;
        }
        case ParamType::Bool: {
            auto* check = new QCheckBox();
            check->setChecked(val.toBool());
            check->setStyleSheet("color: #e0e0e0;");
            check->setProperty("paramName", paramDef.name);
            connect(check, &QCheckBox::toggled, this, [this, check]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        check->property("paramName").toString(), check->isChecked());
                }
            });
            control = check;
            break;
        }
        case ParamType::MultiLineString: {
            auto* edit = new QTextEdit();
            edit->setPlainText(val.toString());
            edit->setMaximumHeight(60);
            edit->setStyleSheet(style);
            edit->setProperty("paramName", paramDef.name);
            connect(edit, &QTextEdit::textChanged, this, [this, edit]() {
                if (!m_currentNodeId.isEmpty()) {
                    m_graph->changeParam(m_currentNodeId,
                        edit->property("paramName").toString(), edit->toPlainText());
                }
            });
            control = edit;
            break;
        }
        case ParamType::IntSlider: {
            auto* w = new QWidget();
            auto* lay = new QHBoxLayout(w);
            lay->setContentsMargins(0, 0, 0, 0);
            int iMin = c.value("min", 0).toInt();
            int iMax = c.value("max", 100).toInt();
            auto* sl = new QSlider(Qt::Horizontal);
            sl->setMinimum(iMin); sl->setMaximum(iMax); sl->setValue(val.toInt());
            auto* sp = new QSpinBox();
            sp->setMinimum(iMin); sp->setMaximum(iMax); sp->setValue(val.toInt());
            sp->setFixedWidth(55); sp->setStyleSheet(style);
            connect(sl, &QSlider::valueChanged, sp, &QSpinBox::setValue);
            connect(sp, QOverload<int>::of(&QSpinBox::valueChanged), sl, &QSlider::setValue);
            sl->setProperty("paramName", paramDef.name);
            connect(sl, &QSlider::valueChanged, this, [this, sl]() {
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, sl->property("paramName").toString(), sl->value());
            });
            lay->addWidget(sl, 1); lay->addWidget(sp);
            control = w;
            break;
        }
        case ParamType::Color: {
            auto* btn = new QPushButton();
            QColor clr;
            QVariantList arr = val.toList();
            if (arr.size() >= 4) clr = QColor(arr[0].toInt(), arr[1].toInt(), arr[2].toInt(), arr[3].toInt());
            else if (arr.size() >= 3) clr = QColor(arr[0].toInt(), arr[1].toInt(), arr[2].toInt());
            else clr = val.value<QColor>();
            btn->setStyleSheet(QString("background-color:%1; border:1px solid #4a4a4a; min-width:28px; min-height:22px;").arg(clr.name(QColor::HexArgb)));
            btn->setProperty("currentColor", clr);
            btn->setProperty("paramName", paramDef.name);
            connect(btn, &QPushButton::clicked, this, [this, btn]() {
                QColor newC = QColorDialog::getColor(btn->property("currentColor").value<QColor>(), this, "", QColorDialog::ShowAlphaChannel);
                if (newC.isValid()) {
                    btn->setProperty("currentColor", newC);
                    btn->setStyleSheet(QString("background-color:%1; border:1px solid #4a4a4a; min-width:28px; min-height:22px;").arg(newC.name(QColor::HexArgb)));
                    if (!m_currentNodeId.isEmpty())
                        m_graph->changeParam(m_currentNodeId, btn->property("paramName").toString(),
                            QVariantList({newC.red(), newC.green(), newC.blue(), newC.alpha()}));
                }
            });
            control = btn;
            break;
        }
        case ParamType::Font: {
            auto* fc = new QFontComboBox();
            fc->setCurrentFont(QFont(val.toString()));
            fc->setStyleSheet(style);
            fc->setProperty("paramName", paramDef.name);
            connect(fc, &QFontComboBox::currentFontChanged, this, [this, fc](const QFont& f) {
                if (!m_currentNodeId.isEmpty())
                    m_graph->changeParam(m_currentNodeId, fc->property("paramName").toString(), f.family());
            });
            control = fc;
            break;
        }
        case ParamType::DoubleSlider: {
            auto* w = new QWidget();
            auto* lay = new QHBoxLayout(w);
            lay->setContentsMargins(0, 0, 0, 0);
            double dMin = c.value("min", 0.0).toDouble(), dMax = c.value("max", 1.0).toDouble();
            auto* sl = new QSlider(Qt::Horizontal);
            sl->setMinimum(0); sl->setMaximum(1000);
            sl->setValue(qRound((val.toDouble() - dMin) / (dMax - dMin) * 1000));
            auto* sp = new QDoubleSpinBox();
            sp->setMinimum(dMin); sp->setMaximum(dMax); sp->setValue(val.toDouble());
            sp->setFixedWidth(55); sp->setStyleSheet(style);
            connect(sl, &QSlider::valueChanged, sp, [=](int v){ sp->setValue(dMin+(dMax-dMin)*v/1000.0); });
            connect(sp, QOverload<double>::of(&QDoubleSpinBox::valueChanged), sl, [=](double v){ sl->setValue(qRound((v-dMin)/(dMax-dMin)*1000)); });
            sl->setProperty("paramName", paramDef.name);
            connect(sl, &QSlider::valueChanged, this, [this, sl, dMin, dMax]() {
                if (!m_currentNodeId.isEmpty()) {
                    double v = dMin + (dMax - dMin) * sl->value() / 1000.0;
                    m_graph->changeParam(m_currentNodeId, sl->property("paramName").toString(), v);
                }
            });
            lay->addWidget(sl, 1); lay->addWidget(sp);
            control = w;
            break;
        }
        default: {
            // 其他复杂类型折叠为一行提示
            auto* lbl = new QLabel(QString("(在属性面板编辑)"));
            lbl->setStyleSheet("color: #808080; font-size: 10px; border: none; font-style: italic;");
            control = lbl;
            break;
        }
        }  // end switch

        if (control) {
            rowLayout->addWidget(label);
            rowLayout->addWidget(control, 1);
            m_paramControls[paramDef.name] = control;
            mainLayout->addWidget(row);
        }
    }

    adjustSize();
}
