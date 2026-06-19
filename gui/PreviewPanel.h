#ifndef SCIFLOW_PREVIEWPANEL_H
#define SCIFLOW_PREVIEWPANEL_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include "../core/NodeBase.h"
#include "../common/NodeData.h"

class PreviewPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewPanel(QWidget *parent = nullptr);
    ~PreviewPanel() override;

    /// 预览指定节点的所有输出端口数据
    void previewNode(NodeBase *node);

    /// 清空所有预览内容
    void clearPreview();

private:
    void setupUI();
    void applyDarkTheme();

    /// 根据数据类型创建对应的预览组件
    QWidget *createPreviewForData(const std::shared_ptr<NodeData>& data);

    QWidget *createImagePreview(const std::shared_ptr<NodeData>& data);
    QWidget *createImageListPreview(const std::shared_ptr<NodeData>& data);
    QWidget *createDataTablePreview(const std::shared_ptr<NodeData>& data);
    QWidget *createNumericPreview(const std::shared_ptr<NodeData>& data);
    QWidget *createTextPreview(const std::shared_ptr<NodeData>& data);

    QTabWidget *m_tabWidget;
    QLabel     *m_emptyLabel;
};

#endif // SCIFLOW_PREVIEWPANEL_H
