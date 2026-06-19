// gui/LegendListEditor.cpp
#include "LegendListEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QColorDialog>

LegendListEditor::LegendListEditor(QWidget* parent) : QWidget(parent)
{
	auto* main=new QVBoxLayout(this); main->setContentsMargins(0,0,0,0);
	m_scrollArea=new QScrollArea; m_scrollArea->setWidgetResizable(true);
	auto* container=new QWidget;
	m_containerLayout=new QVBoxLayout(container);
	m_containerLayout->setContentsMargins(0,0,0,0);
	m_scrollArea->setWidget(container);
	main->addWidget(m_scrollArea,1);
	auto* addBtn=new QPushButton("+ 添加图例"); addBtn->setStyleSheet("QPushButton{background:#4EC9B0;color:#1e1e1e;font-weight:bold;padding:4px 12px;}");
	connect(addBtn,&QPushButton::clicked,this,[this](){
		QVariantMap item; item["color"]=QVariantList{255,0,0,255}; item["label"]="图例"; item["style"]="方块";
		m_items.append(item); rebuildList(); emit valueChanged();
	});
	main->addWidget(addBtn);
}

static QColor fromVar(const QVariant& v){
	QVariantList c=v.toList(); return c.size()>=3?QColor(c[0].toInt(),c[1].toInt(),c[2].toInt()):Qt::red;
}

void LegendListEditor::rebuildList()
{
	QLayoutItem* it;
	while((it=m_containerLayout->takeAt(0))!=nullptr){if(it->widget())it->widget()->deleteLater();delete it;}
	for(int i=0;i<m_items.size();++i){
		QVariantMap d=m_items[i].toMap();
		auto* row=new QWidget; auto* lay=new QHBoxLayout(row); lay->setContentsMargins(2,2,2,2);
		// 颜色按钮
		auto* cb=new QPushButton; QColor clr=fromVar(d["color"]);
		cb->setStyleSheet(QString("background:%1;min-width:24px;min-height:22px;border:1px solid #4a4a4a;").arg(clr.name()));
		cb->setProperty("idx",i);
		connect(cb,&QPushButton::clicked,this,[this,cb,i](){
			QColor nc=QColorDialog::getColor(fromVar(m_items[i].toMap()["color"]),this,"选颜色");
			if(nc.isValid()){QVariantMap m=m_items[i].toMap();m["color"]=QVariantList{nc.red(),nc.green(),nc.blue(),255};m_items[i]=m;cb->setStyleSheet(QString("background:%1;min-width:24px;min-height:22px;border:1px solid #4a4a4a;").arg(nc.name()));emit valueChanged();}
		});
		lay->addWidget(cb);
		// 标签文字
		auto* le=new QLineEdit(d["label"].toString()); le->setMaximumWidth(120); le->setStyleSheet("background:#3a3a3a;color:#e0e0e0;border:1px solid #4a4a4a;");
		le->setProperty("idx",i);
		connect(le,&QLineEdit::textChanged,this,[this,i](const QString& t){QVariantMap m=m_items[i].toMap();m["label"]=t;m_items[i]=m;emit valueChanged();});
		lay->addWidget(le);
		// 样式
		auto* style=new QComboBox; style->addItems({"方块","圆形","线条"});
		style->setCurrentText(d.value("style","方块").toString()); style->setStyleSheet("background:#3a3a3a;color:#e0e0e0;border:1px solid #4a4a4a;");
		connect(style,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this,i,style](){QVariantMap m=m_items[i].toMap();m["style"]=style->currentText();m_items[i]=m;emit valueChanged();});
		lay->addWidget(style);
		// 删除
		auto* del=new QPushButton("×"); del->setFixedSize(22,22); del->setStyleSheet("background:#6e3030;color:#e0e0e0;border:none;");
		connect(del,&QPushButton::clicked,this,[this,i](){m_items.removeAt(i);rebuildList();emit valueChanged();});
		lay->addWidget(del);
		m_containerLayout->addWidget(row);
	}
	m_containerLayout->addStretch();
}

void LegendListEditor::setItems(const QVariantList& items){m_items=items;rebuildList();}
QVariantList LegendListEditor::items() const{return m_items;}
