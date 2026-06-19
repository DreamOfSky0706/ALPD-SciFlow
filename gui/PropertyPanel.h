// gui/PropertyPanel.h
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QMap>

class NodeBase;
class WorkflowGraph;

// 右侧属性面板，展示选中节点的参数
class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(WorkflowGraph* graph, QWidget* parent = nullptr);
    ~PropertyPanel() override = default;

    // 切换到指定节点
    void showNode(const QString& nodeId);
    // 清除显示
    void clearNode();
    // 获取当前节点
    QString currentNodeId() const { return m_currentNodeId; }

    // 面板是否应该展开
    bool shouldExpand() const;

signals:
    void paramChanged(const QString& nodeId, const QString& paramName,
                      const QVariant& newValue);

private slots:
    void onParamControlChanged();

private:
    void rebuildUI();
    QWidget* createControl(const struct ParamDefinition& paramDef,
                           const QVariant& currentValue);

    // 条件可见性更新
    void updateParamVisibility();

    WorkflowGraph* m_graph;
    QString m_currentNodeId;
    NodeBase* m_currentNode = nullptr;

    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;
    QLabel* m_titleLabel;
    QLabel* m_typeLabel;

    // 参数名 → 控件映射
    QMap<QString, QWidget*> m_paramControls;
    QMap<QString, QWidget*> m_paramLabels;

    bool m_expanded = false;

    static constexpr int PANEL_WIDTH = 320;
};
