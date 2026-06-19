#pragma once

#include <QWidget>
#include <QVariantList>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>

// 标注列表编辑器 —— 用于 Annotation 节点
// 管理多个标注项：类型、颜色、线宽、坐标
class AnnotationListEditor : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationListEditor(QWidget *parent = nullptr);

    void setAnnotations(const QVariantList &annotations);
    QVariantList annotations() const;

signals:
    void valueChanged();

private slots:
    void onAddAnnotation();
    void onAnnotationChanged();
    void onDeleteAnnotation(int index);

private:
    // 为单个标注项创建控件组
    QWidget *createAnnotationWidget(int index, const QVariantMap &data);
    void rebuildList();

    QVBoxLayout *m_mainLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_container = nullptr;
    QVBoxLayout *m_containerLayout = nullptr;
    QPushButton *m_addButton = nullptr;

    QVariantList m_annotations;
};
