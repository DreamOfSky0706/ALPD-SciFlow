#include "AnnotationListEditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QColorDialog>

static const QString kDarkStyle = R"(
    QGroupBox {
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        border-radius: 4px;
        margin-top: 8px;
        padding-top: 12px;
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
    }
    QComboBox::drop-down {
        border: none;
    }
    QComboBox QAbstractItemView {
        background-color: #3a3a3a;
        color: #e0e0e0;
        selection-background-color: #4EC9B0;
        selection-color: #1e1e1e;
    }
    QSpinBox, QDoubleSpinBox {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 2px 4px;
        border-radius: 2px;
    }
    QPushButton {
        background-color: #3a3a3a;
        color: #e0e0e0;
        border: 1px solid #4a4a4a;
        padding: 4px 12px;
        border-radius: 2px;
    }
    QPushButton:hover {
        background-color: #4a4a4a;
    }
    QPushButton#addBtn {
        background-color: #4EC9B0;
        color: #1e1e1e;
        font-weight: bold;
    }
    QPushButton#addBtn:hover {
        background-color: #5fd9c0;
    }
    QPushButton#delBtn {
        background-color: #6e3030;
        color: #e0e0e0;
    }
    QPushButton#delBtn:hover {
        background-color: #8e4040;
    }
    QScrollArea {
        border: none;
        background-color: #2b2b2b;
    }
)";

AnnotationListEditor::AnnotationListEditor(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(260, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(4);

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(kDarkStyle);

    m_container = new QWidget;
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(4, 4, 4, 4);
    m_containerLayout->setSpacing(6);
    m_containerLayout->addStretch();

    m_scrollArea->setWidget(m_container);
    m_mainLayout->addWidget(m_scrollArea, 1);

    m_addButton = new QPushButton("+ 添加标注");
    m_addButton->setObjectName("addBtn");
    m_addButton->setStyleSheet(kDarkStyle);
    connect(m_addButton, &QPushButton::clicked, this, &AnnotationListEditor::onAddAnnotation);
    m_mainLayout->addWidget(m_addButton);

    setStyleSheet(kDarkStyle);
}

void AnnotationListEditor::setAnnotations(const QVariantList &annotations)
{
    m_annotations = annotations;
    rebuildList();
}

QVariantList AnnotationListEditor::annotations() const
{
    return m_annotations;
}

// ---- 构建单个标注项的控件 ----

QWidget *AnnotationListEditor::createAnnotationWidget(int index, const QVariantMap &data)
{
    QGroupBox *group = new QGroupBox(QString("标注 %1").arg(index + 1));
    group->setCheckable(true);
    group->setChecked(true);

    QVBoxLayout *groupLayout = new QVBoxLayout(group);

    // ---- 第一行：类型 + 颜色 + 线宽 ----
    QHBoxLayout *row1 = new QHBoxLayout;

    // 类型
    row1->addWidget(new QLabel("类型:"));
    QComboBox *typeCombo = new QComboBox;
    QStringList types = {"箭头", "矩形框", "圆形框", "直线", "文字标签"};
    typeCombo->addItems(types);
    QString curType = data.value("type", "箭头").toString();
    typeCombo->setCurrentText(curType);
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnnotationListEditor::onAnnotationChanged);
    row1->addWidget(typeCombo);

    // 颜色按钮
    row1->addWidget(new QLabel("颜色:"));
    QPushButton *colorBtn = new QPushButton;
    QVariantList c = data.value("color").toList();
    QColor clr = (c.size() >= 3) ? QColor(c[0].toInt(), c[1].toInt(), c[2].toInt()) : QColor(255, 0, 0);
    colorBtn->setStyleSheet(QString("background-color: %1; min-width: 28px; min-height: 22px;"
                                    "border: 1px solid #4a4a4a; border-radius: 2px;")
                                .arg(clr.name()));
    colorBtn->setProperty("annotationIndex", index);
    connect(colorBtn, &QPushButton::clicked, this, [this, index, colorBtn]() {
        QColor oldColor;
        if (index < m_annotations.size()) {
            QVariantMap item = m_annotations[index].toMap();
            QVariantList cc = item.value("color").toList();
            if (cc.size() >= 3)
                oldColor = QColor(cc[0].toInt(), cc[1].toInt(), cc[2].toInt());
            else
                oldColor = QColor(255, 0, 0);
        }
        QColor newColor = QColorDialog::getColor(oldColor, this, "选择颜色");
        if (newColor.isValid() && index < m_annotations.size()) {
            QVariantMap item = m_annotations[index].toMap();
            QVariantList clist;
            clist << newColor.red() << newColor.green() << newColor.blue();
            item["color"] = clist;
            m_annotations[index] = item;
            colorBtn->setStyleSheet(QString("background-color: %1; min-width: 28px; min-height: 22px;"
                                            "border: 1px solid #4a4a4a; border-radius: 2px;")
                                        .arg(newColor.name()));
            emit valueChanged();
        }
    });
    row1->addWidget(colorBtn);

    // 线宽
    row1->addWidget(new QLabel("线宽:"));
    QSpinBox *lwSpin = new QSpinBox;
    lwSpin->setObjectName("lineWidth");
    lwSpin->setRange(1, 20);
    lwSpin->setValue(data.value("lineWidth", 2).toInt());
    connect(lwSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AnnotationListEditor::onAnnotationChanged);
    row1->addWidget(lwSpin);

    row1->addStretch();
    groupLayout->addLayout(row1);

    // ---- 第二行：坐标参数（根据类型不同）----
    QHBoxLayout *row2 = new QHBoxLayout;
    QString annType = data.value("type", "箭头").toString();

    if (annType == "箭头" || annType == "直线") {
        // x1, y1, x2, y2
        row2->addWidget(new QLabel("起点X:"));
        QSpinBox *x1 = new QSpinBox; x1->setObjectName("x1"); x1->setRange(0, 9999); x1->setValue(data.value("x1", 0).toInt());
        connect(x1, QOverload<int>::of(&QSpinBox::valueChanged), this, &AnnotationListEditor::onAnnotationChanged);
        row2->addWidget(x1);

        row2->addWidget(new QLabel("起点Y:"));
        QSpinBox *y1 = new QSpinBox; y1->setObjectName("y1"); y1->setRange(0, 9999); y1->setValue(data.value("y1", 0).toInt());
        connect(y1, QOverload<int>::of(&QSpinBox::valueChanged), this, &AnnotationListEditor::onAnnotationChanged);
        row2->addWidget(y1);

        row2->addWidget(new QLabel("终点X:"));
        QSpinBox *x2 = new QSpinBox; x2->setObjectName("x2"); x2->setRange(0, 9999); x2->setValue(data.value("x2", 0).toInt());
        connect(x2, QOverload<int>::of(&QSpinBox::valueChanged), this, &AnnotationListEditor::onAnnotationChanged);
        row2->addWidget(x2);

        row2->addWidget(new QLabel("终点Y:"));
        QSpinBox *y2 = new QSpinBox; y2->setObjectName("y2"); y2->setRange(0, 9999); y2->setValue(data.value("y2", 0).toInt());
        connect(y2, QOverload<int>::of(&QSpinBox::valueChanged), this, &AnnotationListEditor::onAnnotationChanged);
        row2->addWidget(y2);
    } else if (annType == "矩形框") {
        row2->addWidget(new QLabel("X:")); QSpinBox *x=new QSpinBox; x->setObjectName("x"); x->setRange(0,9999); x->setValue(data.value("x",0).toInt()); connect(x,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(x);
        row2->addWidget(new QLabel("Y:")); QSpinBox *y=new QSpinBox; y->setObjectName("y"); y->setRange(0,9999); y->setValue(data.value("y",0).toInt()); connect(y,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(y);
        row2->addWidget(new QLabel("宽:")); QSpinBox *w=new QSpinBox; w->setObjectName("width"); w->setRange(1,9999); w->setValue(data.value("width",100).toInt()); connect(w,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(w);
        row2->addWidget(new QLabel("高:")); QSpinBox *h=new QSpinBox; h->setObjectName("height"); h->setRange(1,9999); h->setValue(data.value("height",100).toInt()); connect(h,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(h);
    } else if (annType == "圆形框") {
        row2->addWidget(new QLabel("圆心X:")); QSpinBox *cx=new QSpinBox; cx->setObjectName("center_x"); cx->setRange(0,9999); cx->setValue(data.value("center_x",50).toInt()); connect(cx,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(cx);
        row2->addWidget(new QLabel("圆心Y:")); QSpinBox *cy=new QSpinBox; cy->setObjectName("center_y"); cy->setRange(0,9999); cy->setValue(data.value("center_y",50).toInt()); connect(cy,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(cy);
        row2->addWidget(new QLabel("半径:")); QSpinBox *cr=new QSpinBox; cr->setObjectName("radius"); cr->setRange(1,9999); cr->setValue(data.value("radius",30).toInt()); connect(cr,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(cr);
    } else if (annType == "文字标签") {
        row2->addWidget(new QLabel("X:")); QSpinBox *x=new QSpinBox; x->setObjectName("x"); x->setRange(0,9999); x->setValue(data.value("x",0).toInt()); connect(x,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(x);
        row2->addWidget(new QLabel("Y:")); QSpinBox *y=new QSpinBox; y->setObjectName("y"); y->setRange(0,9999); y->setValue(data.value("y",0).toInt()); connect(y,QOverload<int>::of(&QSpinBox::valueChanged),this,&AnnotationListEditor::onAnnotationChanged); row2->addWidget(y);
    }

    row2->addStretch();
    groupLayout->addLayout(row2);

    // ---- 删除按钮 ----
    QPushButton *delBtn = new QPushButton("删除");
    delBtn->setObjectName("delBtn");
    connect(delBtn, &QPushButton::clicked, this, [this, index]() {
        onDeleteAnnotation(index);
    });
    QHBoxLayout *row3 = new QHBoxLayout;
    row3->addStretch();
    row3->addWidget(delBtn);
    groupLayout->addLayout(row3);

    return group;
}

void AnnotationListEditor::rebuildList()
{
    // 清除旧控件（保留 stretch）
    QLayoutItem *item;
    while ((item = m_containerLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    for (int i = 0; i < m_annotations.size(); ++i) {
        QVariantMap data = m_annotations[i].toMap();
        QWidget *w = createAnnotationWidget(i, data);
        m_containerLayout->addWidget(w);
    }

    m_containerLayout->addStretch();
}

// ---- Slots ----

void AnnotationListEditor::onAddAnnotation()
{
    QVariantMap item;
    item["type"] = "箭头";
    item["color"] = QVariantList{255, 0, 0};
    item["lineWidth"] = 2;
    item["x1"] = 0;
    item["y1"] = 0;
    item["x2"] = 100;
    item["y2"] = 100;
    m_annotations.append(item);
    rebuildList();
    emit valueChanged();
}

void AnnotationListEditor::onAnnotationChanged()
{
    // 遍历所有控件，读取值回 m_annotations
    for (int i = 0; i < m_containerLayout->count() - 1; ++i) { // -1 排除 stretch
        QLayoutItem *li = m_containerLayout->itemAt(i);
        if (!li || !li->widget())
            continue;
        QGroupBox *group = qobject_cast<QGroupBox *>(li->widget());
        if (!group)
            continue;

        if (i >= m_annotations.size())
            break;

        QVariantMap item = m_annotations[i].toMap();

        // 读取类型
        QList<QComboBox *> combos = group->findChildren<QComboBox *>();
        for (QComboBox *cb : combos) {
            item["type"] = cb->currentText();
            break; // 只取第一个 combo
        }

        // 通过 objectName 读取各 spin 值
        QList<QSpinBox *> spins = group->findChildren<QSpinBox *>();
        for (QSpinBox *sp : spins) {
            QString name = sp->objectName();
            if (name == "lineWidth")
                item["lineWidth"] = sp->value();
            else if (name == "x1")  item["x1"] = sp->value();
            else if (name == "y1")  item["y1"] = sp->value();
            else if (name == "x2")  item["x2"] = sp->value();
            else if (name == "y2")  item["y2"] = sp->value();
            else if (name == "x")   item["x"] = sp->value();
            else if (name == "y")   item["y"] = sp->value();
            else if (name == "width")  item["width"] = sp->value();
            else if (name == "height") item["height"] = sp->value();
        }

        m_annotations[i] = item;
    }
    emit valueChanged();
}

void AnnotationListEditor::onDeleteAnnotation(int index)
{
    if (index >= 0 && index < m_annotations.size()) {
        m_annotations.removeAt(index);
        rebuildList();
        emit valueChanged();
    }
}
