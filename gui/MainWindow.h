// gui/MainWindow.h
#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QSettings>

class WorkflowGraph;
class NodeGraphicsScene;
class NodeGraphicsView;
class NodeToolbox;
class PropertyPanel;
class FloatingParamPanel;
class PreviewPanel;
class LogPanel;
class ConnectionGraphicsItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    // 文件操作
    void onNewWorkflow();
    void onOpenWorkflow();
    void onSaveWorkflow();
    void onSaveAsWorkflow();

    // 执行
    void onExecuteAll();
    void onExecuteToSelected();
    void onExecuteFromSelected();
    void onStopExecution();

    // 编辑
    void onUndo();
    void onRedo();
    void onCopy();
    void onPaste();
    void onDelete();

    // 视图
    void onAutoLayout();
    void onZoomToFit();
    void onZoomIn();
    void onZoomOut();

    // 节点工具箱
    void onNodeDoubleClicked(const QString& typeName);

    // 场景交互
    void onSceneNodeSelected(const QString& nodeId);
    void onSceneNodeDoubleClicked(const QString& nodeId);
    void onScenePreviewRequested(const QString& nodeId);
    void onSceneSettingsRequested(const QString& nodeId);
    void onSceneSelectionCleared();
    void onSceneStatusMessage(const QString& msg);

    // 右键菜单
    void onNodeContextMenu(const QString& nodeId, const QPointF& scenePos);
    void onCanvasContextMenu(const QPointF& scenePos);
    void onConnectionContextMenu(ConnectionGraphicsItem* conn, const QPointF& scenePos);

    // 执行回调
    void onExecutionStarted();
    void onExecutionProgress(int current, int total, const QString& nodeName);
    void onExecutionFinished(int successCount, int failCount, int skipCount);

    // 缩放变化
    void onZoomChanged(qreal factor);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void createToolbar();
    void createDockWidgets();
    void setupConnections();
    void updateTitle();
    void updateStatusBar();
    bool maybeSave();
    void applyStyleSheet();

    // 核心数据
    WorkflowGraph* m_graph;
    QString m_currentFilePath;
    bool m_modified = false;
    bool m_executing = false;

    // GUI组件
    NodeGraphicsScene* m_scene;
    NodeGraphicsView* m_view;
    NodeToolbox* m_toolbox;
    PropertyPanel* m_propertyPanel;
    FloatingParamPanel* m_floatingPanel;
    PreviewPanel* m_previewPanel;
    LogPanel* m_logPanel;

    // 工具栏控件
    QToolBar* m_toolbar;
    QLabel* m_zoomLabel;
    QProgressBar* m_progressBar;
    QToolButton* m_executeAllAction;
    QAction* m_stopAction;
    QAction* m_undoAction;
    QAction* m_redoAction;

    // 剪贴板（JSON格式存储复制的节点数据）
    QString m_clipboardJson;

    // 设置
    QSettings m_settings;
};
