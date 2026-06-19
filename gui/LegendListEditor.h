// gui/LegendListEditor.h
#pragma once
#include <QWidget>
#include <QVariantList>
#include <QVBoxLayout>
#include <QScrollArea>

class LegendListEditor : public QWidget
{
	Q_OBJECT
public:
	explicit LegendListEditor(QWidget* parent=nullptr);
	void setItems(const QVariantList& items);
	QVariantList items() const;
signals:
	void valueChanged();
private:
	void rebuildList();
	QVariantList m_items;
	QVBoxLayout* m_containerLayout;
	QScrollArea* m_scrollArea;
};
