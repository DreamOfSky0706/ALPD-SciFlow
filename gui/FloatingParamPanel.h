// gui/FloatingParamPanel.h
#pragma once

#include <QWidget>
#include <QMap>

class NodeBase;
class WorkflowGraph;

// 悬浮参数面板（用于简单节点的快速编辑）
class FloatingParamPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingParamPanel(WorkflowGraph* graph, QWidget* parent = nullptr);
    ~FloatingParamPanel() override = default;

    // 在指定位置显示某节点的参数
    void showForNode(const QString& nodeId, const QPoint& screenPos);
    void hide();

    QString currentNodeId() const { return m_currentNodeId; }

signals:
    void paramChanged(const QString& nodeId, const QString& paramName,
                      const QVariant& newValue);

protected:
    bool event(QEvent* event) override;

private:
    void rebuildUI();

    WorkflowGraph* m_graph;
    QString m_currentNodeId;
    NodeBase* m_currentNode = nullptr;

    QMap<QString, QWidget*> m_paramControls;

    static constexpr int MAX_WIDTH = 300;
};
