// gui/MainWindow.cpp
#include "MainWindow.h"
#include "NodeGraphicsScene.h"
#include "NodeGraphicsView.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "NodeToolbox.h"
#include "PropertyPanel.h"
#include "FloatingParamPanel.h"
#include "PreviewPanel.h"
#include "LogPanel.h"
#include "../core/WorkflowGraph.h"
#include "../core/NodeBase.h"
#include "../core/NodeFactory.h"
#include "../io/WorkflowSerializer.h"
#include "../common/Logger.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QSplitter>
#include <QApplication>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QShortcut>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStatusBar>
#include <QTextEdit>
#include <QGraphicsItem>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_settings("SciFlow", "ImageNodeTool")
{
    setWindowTitle("SciFlow");
    resize(1400, 900);

    // 创建核心数据
    m_graph = new WorkflowGraph(this);

    // 创建所有GUI组件
    m_scene = new NodeGraphicsScene(m_graph, this);
    m_view = new NodeGraphicsView(m_scene, this);
    m_toolbox = new NodeToolbox(this);
    m_propertyPanel = new PropertyPanel(m_graph, this);
    m_floatingPanel = new FloatingParamPanel(m_graph, nullptr);
    m_previewPanel = new PreviewPanel(this);
    m_logPanel = new LogPanel(this);

    createToolbar();
    createDockWidgets();
    setupConnections();

    // 快捷键
    auto* delShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(delShortcut, &QShortcut::activated, this, &MainWindow::onDelete);

    auto* undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, this, &MainWindow::onUndo);

    auto* redoShortcut = new QShortcut(QKeySequence(QKeySequence("Ctrl+Shift+Z")), this);
    connect(redoShortcut, &QShortcut::activated, this, &MainWindow::onRedo);

    auto* copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, [this]() {
        // 若焦点在文本框内则不拦截，让文本框自行处理复制
        if (qobject_cast<QTextEdit*>(QApplication::focusWidget()))
            return;
        onCopy();
    });

    auto* pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindow::onPaste);

    auto* newShortcut = new QShortcut(QKeySequence::New, this);
    connect(newShortcut, &QShortcut::activated, this, &MainWindow::onNewWorkflow);

    auto* openShortcut = new QShortcut(QKeySequence::Open, this);
    connect(openShortcut, &QShortcut::activated, this, &MainWindow::onOpenWorkflow);

    auto* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSaveWorkflow);

    auto* saveAsShortcut = new QShortcut(QKeySequence("Ctrl+Shift+S"), this);
    connect(saveAsShortcut, &QShortcut::activated, this, &MainWindow::onSaveAsWorkflow);

    auto* f5Shortcut = new QShortcut(QKeySequence("F5"), this);
    connect(f5Shortcut, &QShortcut::activated, this, &MainWindow::onExecuteAll);

    auto* selAllShortcut = new QShortcut(QKeySequence::SelectAll, this);
    connect(selAllShortcut, &QShortcut::activated, this, [this]() {
        for (auto* item : m_scene->items()) {
            item->setSelected(true);
        }
    });

    auto* spaceShortcut = new QShortcut(QKeySequence("Space"), this);
    connect(spaceShortcut, &QShortcut::activated, this, [this]() {
        auto selected = m_scene->selectedItems();
        for (auto* item : selected) {
            if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
                onScenePreviewRequested(nodeItem->nodeId());
                break;
            }
        }
    });

    auto* escShortcut = new QShortcut(QKeySequence("Escape"), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        m_floatingPanel->hide();
        m_scene->clearSelection();
    });

    updateStatusBar();
    Logger::instance().info("SciFlow 启动完成");
    updateTitle();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolbar()
{
    m_toolbar = addToolBar("主工具栏");
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(20, 20));

    QAction* newAction = m_toolbar->addAction("新建");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewWorkflow);

    QAction* openAction = m_toolbar->addAction("打开");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenWorkflow);

    QAction* saveAction = m_toolbar->addAction("保存");
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveWorkflow);

    QAction* saveAsAction = m_toolbar->addAction("另存为");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsWorkflow);

    m_toolbar->addSeparator();

    // 执行按钮（带下拉菜单）
    m_executeAllAction = new QToolButton();
    m_executeAllAction->setText("  ▶ 执行  ");
    m_executeAllAction->setToolTip("执行全部 (F5)");
    m_executeAllAction->setPopupMode(QToolButton::MenuButtonPopup);
    m_executeAllAction->setStyleSheet("QToolButton { padding-right: 16px; }");
    QMenu* execMenu = new QMenu(m_executeAllAction);
    execMenu->addAction("执行全部", this, &MainWindow::onExecuteAll, QKeySequence("F5"));
    execMenu->addAction("执行到选中节点", this, &MainWindow::onExecuteToSelected);
    execMenu->addAction("从选中节点执行到末端", this, &MainWindow::onExecuteFromSelected);
    m_executeAllAction->setMenu(execMenu);
    m_executeAllAction->setDefaultAction(execMenu->actions().first());
    m_toolbar->addWidget(m_executeAllAction);

    m_stopAction = m_toolbar->addAction("■ 停止");
    m_stopAction->setEnabled(false);
    connect(m_stopAction, &QAction::triggered, this, &MainWindow::onStopExecution);

    m_toolbar->addSeparator();

    m_undoAction = m_toolbar->addAction("撤销");
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::onUndo);

    m_redoAction = m_toolbar->addAction("重做");
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::onRedo);

    m_toolbar->addSeparator();

    QAction* autoLayoutAction = m_toolbar->addAction("自动布局");
    connect(autoLayoutAction, &QAction::triggered, this, &MainWindow::onAutoLayout);

    QAction* zoomFitAction = m_toolbar->addAction("适配");
    connect(zoomFitAction, &QAction::triggered, this, &MainWindow::onZoomToFit);

    m_toolbar->addSeparator();

    QAction* zoomOutAction = m_toolbar->addAction("−");
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::onZoomOut);

    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setStyleSheet("color: #e0e0e0; min-width: 40px;");
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_toolbar->addWidget(m_zoomLabel);

    QAction* zoomInAction = m_toolbar->addAction("+");
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::onZoomIn);

    m_toolbar->addSeparator();

    // 进度条
    m_progressBar = new QProgressBar();
    m_progressBar->setFixedWidth(150);
    m_progressBar->setMaximumHeight(20);
    m_progressBar->setVisible(false);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #3a3a3a; border: 1px solid #4a4a4a; text-align: center; color: #e0e0e0; }"
        "QProgressBar::chunk { background-color: #4EC9B0; }");
    m_toolbar->addWidget(m_progressBar);
}

void MainWindow::createDockWidgets()
{
    // 用水平QSplitter替代QDockWidget布局，避免Windows下原生窗口闪烁
    auto* hSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧工具箱
    auto* leftPanel = new QWidget();
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0,0,0,0);
    auto* toolboxLabel = new QLabel("节点工具箱");
    toolboxLabel->setStyleSheet("color:#e0e0e0;background:#333;padding:4px 8px;font-weight:bold;");
    leftLayout->addWidget(toolboxLabel);
    leftLayout->addWidget(m_toolbox, 1);
    leftPanel->setMinimumWidth(180);
    leftPanel->setMaximumWidth(300);
    hSplitter->addWidget(leftPanel);

    // 中央画布
    hSplitter->addWidget(m_view);

    // 右侧属性面板
    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0,0,0,0);
    auto* propLabel = new QLabel("属性");
    propLabel->setStyleSheet("color:#e0e0e0;background:#333;padding:4px 8px;font-weight:bold;");
    rightLayout->addWidget(propLabel);
    rightLayout->addWidget(m_propertyPanel, 1);
    rightPanel->setMinimumWidth(260);
    hSplitter->addWidget(rightPanel);

    hSplitter->setStretchFactor(0, 0);  // 左：固定
    hSplitter->setStretchFactor(1, 1);  // 中：拉伸
    hSplitter->setStretchFactor(2, 0);  // 右：固定
    hSplitter->setSizes({220, 800, 320});
    setCentralWidget(hSplitter);

    // 底部预览+日志（仍用QDockWidget因为不频繁重建控件）
    auto* bottomDock = new QDockWidget("预览与日志", this);
    auto* bottomSplitter = new QSplitter(Qt::Horizontal);

    // 预览面板
    auto* previewWidget = new QWidget();
    auto* previewLayout = new QVBoxLayout(previewWidget);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    auto* previewLabel = new QLabel("预览");
    previewLabel->setStyleSheet("color: #e0e0e0; background-color: #333333; padding: 4px 8px; font-weight: bold;");
    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(m_previewPanel, 1);
    bottomSplitter->addWidget(previewWidget);

    // 日志面板
    auto* logWidget = new QWidget();
    auto* logLayout = new QVBoxLayout(logWidget);
    logLayout->setContentsMargins(0, 0, 0, 0);
    auto* logLabel = new QLabel("日志");
    logLabel->setStyleSheet("color: #e0e0e0; background-color: #333333; padding: 4px 8px; font-weight: bold;");
    logLayout->addWidget(logLabel);
    logLayout->addWidget(m_logPanel, 1);
    bottomSplitter->addWidget(logWidget);

    bottomSplitter->setStretchFactor(0, 3);
    bottomSplitter->setStretchFactor(1, 2);
    bottomDock->setWidget(bottomSplitter);
    bottomDock->setFeatures(QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock);
}

void MainWindow::setupConnections()
{
    // 工具箱双击添加节点
    connect(m_toolbox, &NodeToolbox::nodeDoubleClicked,
            this, &MainWindow::onNodeDoubleClicked);

    // 场景交互
    connect(m_scene, &NodeGraphicsScene::nodeSelected,
            this, &MainWindow::onSceneNodeSelected);
    connect(m_scene, &NodeGraphicsScene::nodeDoubleClicked,
            this, &MainWindow::onSceneNodeDoubleClicked);
    connect(m_scene, &NodeGraphicsScene::previewRequested,
            this, &MainWindow::onScenePreviewRequested);
    connect(m_scene, &NodeGraphicsScene::settingsRequested,
            this, &MainWindow::onSceneSettingsRequested);
    connect(m_scene, &NodeGraphicsScene::selectionCleared,
            this, &MainWindow::onSceneSelectionCleared);
    connect(m_scene, &NodeGraphicsScene::statusMessage,
            this, &MainWindow::onSceneStatusMessage);

    // 右键菜单
    connect(m_scene, &NodeGraphicsScene::nodeContextMenuRequested,
            this, &MainWindow::onNodeContextMenu);
    connect(m_scene, &NodeGraphicsScene::canvasContextMenuRequested,
            this, &MainWindow::onCanvasContextMenu);
    connect(m_scene, &NodeGraphicsScene::connectionContextMenuRequested,
            this, &MainWindow::onConnectionContextMenu);

    // 执行回调
    connect(m_graph, &WorkflowGraph::executionStarted,
            this, &MainWindow::onExecutionStarted);
    connect(m_graph, &WorkflowGraph::executionProgress,
            this, &MainWindow::onExecutionProgress);
    connect(m_graph, &WorkflowGraph::executionFinished,
            this, &MainWindow::onExecutionFinished);

    // 节点/连线变更触发标题和状态栏刷新
    connect(m_graph, &WorkflowGraph::nodeAdded, this, [this](const QString&) {
        m_modified = true; updateTitle(); });
    connect(m_graph, &WorkflowGraph::nodeRemoved, this, [this](const QString&) {
        m_modified = true; updateTitle(); });
    connect(m_graph, &WorkflowGraph::connectionAdded, this, [this](const Connection&) {
        m_modified = true; updateTitle(); });
    connect(m_graph, &WorkflowGraph::connectionRemoved, this, [this](const Connection&) {
        m_modified = true; updateTitle(); });

    // 缩放
    connect(m_view, &NodeGraphicsView::zoomChanged,
            this, &MainWindow::onZoomChanged);

    // 属性面板参数修改
    connect(m_propertyPanel, &PropertyPanel::paramChanged,
            this, [this](const QString& nodeId, const QString& paramName, const QVariant& value) {
                Q_UNUSED(nodeId) Q_UNUSED(paramName) Q_UNUSED(value)
                m_modified = true;
                updateTitle();
            });
}

void MainWindow::updateTitle()
{
    QString title = "SciFlow";
    if (!m_currentFilePath.isEmpty()) {
        title += " - " + QFileInfo(m_currentFilePath).fileName();
    }
    if (m_modified) {
        title += " *";
    }
    setWindowTitle(title);

    // 状态栏显示节点和连线计数
    int nodeCount = m_graph->allNodes().size();
    int connCount = m_graph->allConnections().size();
    statusBar()->showMessage(QString("节点: %1 | 连线: %2").arg(nodeCount).arg(connCount));
}

void MainWindow::updateStatusBar()
{
    int nodeCount = m_graph->allNodes().size();
    int connCount = m_graph->allConnections().size();
    statusBar()->showMessage(QString("节点: %1 | 连线: %2").arg(nodeCount).arg(connCount));
}

bool MainWindow::maybeSave()
{
    if (!m_modified) return true;

    QMessageBox box(this);
    box.setWindowTitle("保存");
    box.setText("当前工作流有未保存的修改，是否保存？");
    box.setIcon(QMessageBox::Question);
    QPushButton* saveBtn = box.addButton("保存", QMessageBox::AcceptRole);
    QPushButton* discardBtn = box.addButton("不保存", QMessageBox::DestructiveRole);
    box.addButton("取消", QMessageBox::RejectRole);
    box.setDefaultButton(saveBtn);
    box.exec();
    if (box.clickedButton() == saveBtn) {
        onSaveWorkflow();
        return !m_modified;
    } else if (box.clickedButton() == discardBtn) {
        return true;
    } else {
        return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// ========== 文件操作 ==========

void MainWindow::onNewWorkflow()
{
    if (!maybeSave()) return;

    m_graph->clear();
    m_currentFilePath.clear();
    m_modified = false;
    m_previewPanel->clearPreview();
    Logger::instance().info("新建工作流");
    updateStatusBar();
    updateTitle();
}

void MainWindow::onOpenWorkflow()
{
    if (!maybeSave()) return;

    QString path = QFileDialog::getOpenFileName(this, "打开工作流",
        m_settings.value("lastDir", "").toString(),
        "工作流文件 (*.flow.json);;所有文件 (*)");

    if (path.isEmpty()) return;

    m_settings.setValue("lastDir", QFileInfo(path).absolutePath());

    // clear() 会发射信号让Scene自动清理图形项
    m_graph->clear();
    if (!WorkflowSerializer::load(path, m_graph)) {
        QMessageBox::critical(this, "加载失败", "无法加载工作流文件");
        Logger::instance().error("加载工作流失败");
        return;
    }

    m_currentFilePath = path;
    m_modified = false;

    // 图形项已由 WorkflowGraph 的信号自动创建，无需手动添加

    Logger::instance().success("加载工作流: " + QFileInfo(path).fileName());
    updateStatusBar();
    updateTitle();
}

void MainWindow::onSaveWorkflow()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveAsWorkflow();
        return;
    }

    if (!WorkflowSerializer::save(m_graph, m_currentFilePath)) {
        QMessageBox::critical(this, "保存失败", "无法保存工作流文件");
        Logger::instance().error("保存工作流失败");
        return;
    }

    m_modified = false;
    Logger::instance().success("保存工作流: " + QFileInfo(m_currentFilePath).fileName());
    statusBar()->showMessage("已保存: " + m_currentFilePath);
    updateTitle();
}

void MainWindow::onSaveAsWorkflow()
{
    QString path = QFileDialog::getSaveFileName(this, "另存为",
        m_settings.value("lastDir", "").toString() + "/untitled.flow.json",
        "工作流文件 (*.flow.json);;所有文件 (*)");

    if (path.isEmpty()) return;

    m_settings.setValue("lastDir", QFileInfo(path).absolutePath());

    if (!WorkflowSerializer::save(m_graph, path)) {
        QMessageBox::critical(this, "保存失败", "无法保存工作流文件");
        Logger::instance().error("保存工作流失败");
        return;
    }

    m_currentFilePath = path;
    m_modified = false;
    Logger::instance().success("另存为: " + QFileInfo(path).fileName());
    statusBar()->showMessage("已另存为: " + path);
    updateTitle();
}

// ========== 执行 ==========

void MainWindow::onExecuteAll()
{
    if (m_graph->allNodes().isEmpty()) {
        statusBar()->showMessage("画布为空，无节点可执行");
        return;
    }
    m_executing = true;
    m_executeAllAction->setEnabled(false);
    m_stopAction->setEnabled(true);
    m_graph->executeAll();
}

void MainWindow::onExecuteToSelected()
{
    auto selected = m_scene->selectedItems();
    for (auto* item : selected) {
        if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
            m_graph->executePartial(nodeItem->nodeId(),
                WorkflowGraph::PartialDirection::Upstream);
            statusBar()->showMessage("已执行到选中节点");
            return;
        }
    }
    statusBar()->showMessage("未选中任何节点");
}

void MainWindow::onExecuteFromSelected()
{
    auto selected = m_scene->selectedItems();
    for (auto* item : selected) {
        if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
            m_graph->executePartial(nodeItem->nodeId(),
                WorkflowGraph::PartialDirection::Downstream);
            statusBar()->showMessage("已从选中节点执行到末端");
            return;
        }
    }
    statusBar()->showMessage("未选中任何节点");
}

void MainWindow::onStopExecution()
{
    m_executing = false;
    m_executeAllAction->setEnabled(true);
    m_stopAction->setEnabled(false);
    m_progressBar->setVisible(false);
    statusBar()->showMessage("执行已停止");
}

// ========== 编辑 ==========

void MainWindow::onUndo()
{
    if (m_graph->undoStack()->canUndo()) {
        m_graph->undoStack()->undo();
        m_modified = true;
        updateStatusBar();
        updateTitle();
    }
}

void MainWindow::onRedo()
{
    if (m_graph->undoStack()->canRedo()) {
        m_graph->undoStack()->redo();
        m_modified = true;
        updateStatusBar();
        updateTitle();
    }
}

void MainWindow::onCopy()
{
    auto selected = m_scene->selectedItems();
    QJsonArray nodesArr;
    QSet<QString> copiedIds;

    for (auto* item : selected) {
        if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
            NodeBase* node = nodeItem->node();
            QJsonObject nodeObj;
            nodeObj["type"] = node->typeName();
            nodeObj["display_name"] = node->displayName();
            QJsonObject posObj;
            posObj["x"] = nodeItem->pos().x();
            posObj["y"] = nodeItem->pos().y();
            nodeObj["position"] = posObj;

            QJsonObject paramsObj;
            for (const auto& pd : node->paramDefinitions()) {
                QVariant val = node->param(pd.name);
                // 序列化参数值
                if (val.type() == QVariant::List) {
                    QJsonArray arr;
                    for (const auto& v : val.toList()) {
                        arr.append(QJsonValue::fromVariant(v));
                    }
                    paramsObj[pd.name] = arr;
                } else {
                    paramsObj[pd.name] = QJsonValue::fromVariant(val);
                }
            }
            nodeObj["params"] = paramsObj;
            nodeObj["_oldId"] = node->id(); // 临时ID用于重建连线
            nodesArr.append(nodeObj);
            copiedIds.insert(node->id());
        }
    }

    QJsonObject clipObj;
    clipObj["nodes"] = nodesArr;

    // 复制两端都在选中范围内的连线
    QJsonArray connsArr;
    for (const auto& conn : m_graph->allConnections()) {
        if (copiedIds.contains(conn.sourceNodeId()) &&
            copiedIds.contains(conn.targetNodeId())) {
            QJsonObject connObj;
            connObj["from_node"] = conn.sourceNodeId();
            connObj["from_port"] = conn.sourcePortName();
            connObj["to_node"] = conn.targetNodeId();
            connObj["to_port"] = conn.targetPortName();
            connsArr.append(connObj);
        }
    }
    clipObj["connections"] = connsArr;

    m_clipboardJson = QJsonDocument(clipObj).toJson(QJsonDocument::Compact);
    QApplication::clipboard()->setText(m_clipboardJson);
    statusBar()->showMessage(QString("已复制 %1 个节点").arg(nodesArr.size()));
}

void MainWindow::onPaste()
{
    m_clipboardJson = QApplication::clipboard()->text();
    if (m_clipboardJson.isEmpty()) return;

    QJsonDocument doc = QJsonDocument::fromJson(m_clipboardJson.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QJsonArray nodesArr = obj["nodes"].toArray();
    if (nodesArr.isEmpty()) return;

    // 创建新节点
    QMap<QString, QString> idMapping; // oldId → newId
    QPointF offset(30, 30);

    for (const auto& nodeVal : nodesArr) {
        QJsonObject nodeObj = nodeVal.toObject();
        QString typeName = nodeObj["type"].toString();
        QString oldId = nodeObj["_oldId"].toString();

        // 计算偏移位置
        QPointF pos(nodeObj["position"].toObject()["x"].toDouble() + offset.x(),
                    nodeObj["position"].toObject()["y"].toDouble() + offset.y());

        // 传入位置给addNode，信号触发时图形项就会在正确位置
        NodeBase* newNode = m_graph->addNode(typeName, pos);
        if (!newNode) continue;

        idMapping[oldId] = newNode->id();

        // 设置参数
        QJsonObject paramsObj = nodeObj["params"].toObject();
        for (auto it = paramsObj.begin(); it != paramsObj.end(); ++it) {
            QVariant val = it.value().toVariant();
            newNode->setParam(it.key(), val);
        }

        // 图形项已由信号自动创建
    }

    // 重建连线
    QJsonArray connsArr = obj["connections"].toArray();
    for (const auto& connVal : connsArr) {
        QJsonObject connObj = connVal.toObject();
        QString newSrc = idMapping.value(connObj["from_node"].toString());
        QString newDst = idMapping.value(connObj["to_node"].toString());
        if (!newSrc.isEmpty() && !newDst.isEmpty()) {
            m_graph->addConnection(newSrc, connObj["from_port"].toString(),
                                    newDst, connObj["to_port"].toString());
        }
    }

    m_modified = true;
    updateStatusBar();
    updateTitle();
    Logger::instance().info(QString("粘贴了 %1 个节点").arg(nodesArr.size()));
}

void MainWindow::onDelete()
{
    // 先收集所有待删除的节点ID和连线信息，避免在迭代中删除导致崩溃
    QStringList nodeIdsToDelete;
    QList<Connection> connsToDelete;

    auto selected = m_scene->selectedItems();
    for (auto* item : selected) {
        if (auto* connItem = dynamic_cast<ConnectionGraphicsItem*>(item)) {
            connsToDelete.append(connItem->toConnection());
        } else if (auto* nodeItem = dynamic_cast<NodeGraphicsItem*>(item)) {
            nodeIdsToDelete.append(nodeItem->nodeId());
        }
    }

    // 清除UI中对这些节点的所有引用
    m_scene->clearSelection();
    m_propertyPanel->clearNode();
    m_previewPanel->clearPreview();

    // 先删除连线
    for (const auto& conn : connsToDelete) {
        m_graph->removeConnection(conn.sourceNodeId(), conn.sourcePortName(),
                                   conn.targetNodeId(), conn.targetPortName());
    }

    // 再删除节点（需检查节点是否仍存在，可能已被连线删除的副作用移除）
    for (const auto& nodeId : nodeIdsToDelete) {
        if (m_graph->nodeById(nodeId)) {
            m_graph->removeNode(nodeId);
        }
    }

    if (!nodeIdsToDelete.isEmpty() || !connsToDelete.isEmpty()) {
        m_modified = true;
        updateStatusBar();
        updateTitle();
    }
}

// ========== 视图 ==========

void MainWindow::onAutoLayout()
{
    m_view->autoLayoutNodes();
    statusBar()->showMessage("自动布局完成");
}

void MainWindow::onZoomToFit()
{
    m_view->zoomToFit();
}

void MainWindow::onZoomIn()
{
    m_view->zoomIn();
}

void MainWindow::onZoomOut()
{
    m_view->zoomOut();
}

// ========== 工具箱 ==========

void MainWindow::onNodeDoubleClicked(const QString& typeName)
{
    // 在视口中心添加节点
    QPointF centerPos = m_view->mapToScene(m_view->viewport()->rect().center());

    NodeBase* node = m_graph->addNode(typeName, centerPos);
    if (!node) {
        Logger::instance().error("无法创建节点类型: " + typeName);
        return;
    }

    // 图形项已由信号自动创建
    m_toolbox->addToRecent(typeName);
    m_modified = true;
    updateStatusBar();
    updateTitle();
    Logger::instance().info(QString("添加节点: %1").arg(node->displayName()));
}

// ========== 场景回调 ==========

void MainWindow::onSceneNodeSelected(const QString& nodeId)
{
    // 选中时仅记录节点ID，不在拖拽过程中弹出或刷新面板
    // 属性面板的刷新由用户显式操作触发（双击/设置按钮/右键）
    Q_UNUSED(nodeId)
}

void MainWindow::onSceneNodeDoubleClicked(const QString& nodeId)
{
    NodeBase* node = m_graph->nodeById(nodeId);
    if (node) {
        QPoint screenPos = QCursor::pos();
        m_floatingPanel->showForNode(nodeId, screenPos);
    }
}

void MainWindow::onScenePreviewRequested(const QString& nodeId)
{
    NodeBase* node = m_graph->nodeById(nodeId);
    if (!node) return;
    // 先执行上游节点确保该节点有输出数据
    m_graph->executePartial(nodeId, WorkflowGraph::PartialDirection::Upstream);
    m_previewPanel->previewNode(node);
    statusBar()->showMessage(QString("预览节点: %1").arg(node->displayName()));
}

void MainWindow::onSceneSettingsRequested(const QString& nodeId)
{
    m_propertyPanel->showNode(nodeId);
}

void MainWindow::onSceneSelectionCleared()
{
    m_propertyPanel->clearNode();
}

void MainWindow::onSceneStatusMessage(const QString& msg)
{
    statusBar()->showMessage(msg);
}

// ========== 右键菜单 ==========

void MainWindow::onNodeContextMenu(const QString& nodeId, const QPointF& scenePos)
{
    Q_UNUSED(scenePos)
    if (nodeId.isEmpty()) return;

    QMenu menu;
    menu.setStyleSheet(
        "QMenu { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #4a4a4a; }"
        "QMenu::item { padding: 6px 30px 6px 12px; }"
        "QMenu::item:selected { background-color: #4EC9B0; color: #1e1e1e; }");

    QAction* previewAction = menu.addAction("预览输出");
    QAction* renameAction = menu.addAction("重命名");
    menu.addSeparator();
    QAction* execToAction = menu.addAction("执行到此节点");
    QAction* execFromAction = menu.addAction("从此节点执行到末端");
    menu.addSeparator();
    QAction* copyAction = menu.addAction("复制");
    QAction* deleteAction = menu.addAction("删除");
    QAction* disconnectAction = menu.addAction("断开所有连线");

    QAction* chosen = menu.exec(QCursor::pos());
    if (!chosen) return;

    if (chosen == previewAction) {
        onScenePreviewRequested(nodeId);
    } else if (chosen == execToAction) {
        m_graph->executePartial(nodeId, WorkflowGraph::PartialDirection::Upstream);
        statusBar()->showMessage("已执行到选中节点");
    } else if (chosen == execFromAction) {
        m_graph->executePartial(nodeId, WorkflowGraph::PartialDirection::Downstream);
        statusBar()->showMessage("已从选中节点执行到末端");
    } else if (chosen == renameAction) {
        NodeBase* node = m_graph->nodeById(nodeId);
        if (!node) return;
        bool ok;
        QString newName = QInputDialog::getText(this, "重命名节点",
            "节点名称:", QLineEdit::Normal, node->displayName(), &ok);
        if (ok && !newName.isEmpty()) {
            node->setDisplayName(newName);
            m_modified = true;
            updateTitle();
            if (auto* item = m_scene->nodeItemById(nodeId))
                item->update();
        }
    } else if (chosen == copyAction) {
        // 选中节点后复制
        if (auto* item = m_scene->nodeItemById(nodeId)) {
            item->setSelected(true);
            onCopy();
        }
    } else if (chosen == deleteAction) {
        m_propertyPanel->clearNode();  // 先清除属性面板引用
        m_previewPanel->clearPreview();  // 清除预览
        m_graph->removeNode(nodeId);
        m_modified = true;
        updateStatusBar();
        updateTitle();
    } else if (chosen == disconnectAction) {
        m_graph->removeConnectionsOfNode(nodeId);
        m_modified = true;
        updateStatusBar();
        updateTitle();
    }
}

void MainWindow::onCanvasContextMenu(const QPointF& scenePos)
{
    QMenu menu;
    menu.setStyleSheet(
        "QMenu { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #4a4a4a; }"
        "QMenu::item { padding: 6px 30px 6px 12px; }"
        "QMenu::item:selected { background-color: #4EC9B0; color: #1e1e1e; }");

    // "添加节点"子菜单
    QMenu* addNodeMenu = menu.addMenu("添加节点");
    const auto& regs = NodeFactory::instance().allRegistrations();

    // 按分类组织
    QMap<QString, QMenu*> categoryMenus;
    for (const auto& reg : regs) {
        QStringList parts = reg.categoryPath.split("/");
        QString category = parts.isEmpty() ? "其他" : parts.first();

        if (!categoryMenus.contains(category)) {
            categoryMenus[category] = addNodeMenu->addMenu(category);
            categoryMenus[category]->setStyleSheet(menu.styleSheet());
        }

        QAction* nodeAction = categoryMenus[category]->addAction(reg.displayName);
        QString typeName = reg.typeName;
        connect(nodeAction, &QAction::triggered, this, [this, typeName, scenePos]() {
            NodeBase* node = m_graph->addNode(typeName, scenePos);
            if (node) {
                // 图形项已由信号自动创建
                m_toolbox->addToRecent(typeName);
                m_modified = true;
                updateStatusBar();
                updateTitle();
            }
        });
    }

    menu.addSeparator();

    QAction* pasteAction = menu.addAction("粘贴");
    pasteAction->setEnabled(!QApplication::clipboard()->text().isEmpty());
    connect(pasteAction, &QAction::triggered, this, &MainWindow::onPaste);

    menu.addSeparator();

    QAction* fitAction = menu.addAction("缩放适配");
    connect(fitAction, &QAction::triggered, this, &MainWindow::onZoomToFit);

    QAction* layoutAction = menu.addAction("自动布局整理");
    connect(layoutAction, &QAction::triggered, this, &MainWindow::onAutoLayout);

    menu.exec(QCursor::pos());
}

void MainWindow::onConnectionContextMenu(ConnectionGraphicsItem* conn, const QPointF& scenePos)
{
    Q_UNUSED(scenePos)
    if (!conn) return;

    QMenu menu;
    menu.setStyleSheet(
        "QMenu { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #4a4a4a; }"
        "QMenu::item { padding: 6px 30px 6px 12px; }"
        "QMenu::item:selected { background-color: #4EC9B0; color: #1e1e1e; }");

    QAction* deleteAction = menu.addAction("删除连线");

    QAction* chosen = menu.exec(QCursor::pos());
    if (chosen == deleteAction) {
        auto c = conn->toConnection();
        m_graph->removeConnection(c.sourceNodeId(), c.sourcePortName(),
                                   c.targetNodeId(), c.targetPortName());
        m_modified = true;
        updateStatusBar();
        updateTitle();
    }
}

// ========== 执行回调 ==========

void MainWindow::onExecutionStarted()
{
    m_executing = true;
    m_executeAllAction->setEnabled(false);
    m_stopAction->setEnabled(true);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    statusBar()->showMessage("执行中...");
}

void MainWindow::onExecutionProgress(int current, int total, const QString& nodeName)
{
    m_progressBar->setMaximum(total);
    m_progressBar->setValue(current);
    statusBar()->showMessage(QString("执行中: %1 [%2/%3]").arg(nodeName).arg(current).arg(total));
}

void MainWindow::onExecutionFinished(int successCount, int failCount, int skipCount)
{
    m_executing = false;
    m_executeAllAction->setEnabled(true);
    m_stopAction->setEnabled(false);
    m_progressBar->setVisible(false);
    statusBar()->showMessage(QString("执行完毕: 成功%1 失败%2 跳过%3")
        .arg(successCount).arg(failCount).arg(skipCount));
    m_modified = true;
    updateTitle();
}

// ========== 缩放 ==========

void MainWindow::onZoomChanged(qreal factor)
{
    m_zoomLabel->setText(QString("%1%").arg(qRound(factor * 100)));
}
