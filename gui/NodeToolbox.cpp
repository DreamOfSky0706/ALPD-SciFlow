#include "NodeToolbox.h"

#include "../core/NodeFactory.h"
#include "../core/NodeBase.h"
#include "../common/NodeData.h"
#include "../common/DataType.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QDrag>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QRegularExpression>
#include <QApplication>
#include <QHeaderView>
#include <QCursor>

// ---------------------------------------------------------------------------
// MIME 类型常量
// ---------------------------------------------------------------------------
const QString NodeToolbox::MimeType = QStringLiteral("application/x-sciflow-nodetype");

// ---------------------------------------------------------------------------
// 内部：支持拖拽的 QTreeWidget 子类
// ---------------------------------------------------------------------------
class DragEnabledTreeWidget : public QTreeWidget
{
    QPoint m_dragStartPos;

public:
    explicit DragEnabledTreeWidget(QWidget *parent = nullptr)
        : QTreeWidget(parent)
    {
        setDragEnabled(true);
        setDragDropMode(QAbstractItemView::DragOnly);
        setSelectionMode(QAbstractItemView::SingleSelection);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton)
            m_dragStartPos = event->pos();
        QTreeWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton) {
            int distance = (event->pos() - m_dragStartPos).manhattanLength();
            if (distance >= QApplication::startDragDistance()) {
                QTreeWidgetItem *item = itemAt(m_dragStartPos);
                if (item && item->childCount() == 0) {
                    // 只拖拽叶子节点
                    QString typeName = item->data(0, Qt::UserRole).toString();
                    if (!typeName.isEmpty()) {
                        QDrag *drag = new QDrag(this);
                        QMimeData *mime = new QMimeData();
                        mime->setData(NodeToolbox::MimeType, typeName.toUtf8());
                        drag->setMimeData(mime);

                        // 拖拽预览图标使用当前项的图标
                        QPixmap pix = item->icon(0).pixmap(24, 24);
                        if (!pix.isNull())
                            drag->setPixmap(pix);

                        drag->exec(Qt::CopyAction);
                        return;
                    }
                }
            }
        }
        QTreeWidget::mouseMoveEvent(event);
    }
};

// ---------------------------------------------------------------------------
// 内部：支持拖入的 QListWidget（用于收藏夹接收节点类型拖放）
// ---------------------------------------------------------------------------
class FavoriteDropListWidget : public QListWidget
{
public:
    explicit FavoriteDropListWidget(QWidget* parent, NodeToolbox* toolbox)
        : QListWidget(parent), m_toolbox(toolbox)
    {
        setAcceptDrops(true);
        setDragEnabled(false);
        setDefaultDropAction(Qt::CopyAction);
    }

protected:
    void dragEnterEvent(QDragEnterEvent* event) override
    {
        if (event->mimeData()->hasFormat(NodeToolbox::MimeType))
            event->acceptProposedAction();
        else
            event->ignore();
    }
    void dragMoveEvent(QDragMoveEvent* event) override
    {
        if (event->mimeData()->hasFormat(NodeToolbox::MimeType))
            event->acceptProposedAction();
    }
    void dropEvent(QDropEvent* event) override
    {
        if (event->mimeData()->hasFormat(NodeToolbox::MimeType)) {
            QString typeName = QString::fromUtf8(event->mimeData()->data(NodeToolbox::MimeType));
            if (!typeName.isEmpty())
                m_toolbox->addToFavorite(typeName);
            event->acceptProposedAction();
        }
    }

private:
    NodeToolbox* m_toolbox;
};

// ===================================================================
// NodeToolbox 实现
// ===================================================================

NodeToolbox::NodeToolbox(QWidget *parent)
    : QWidget(parent)
    , m_searchBox(nullptr)
    , m_recentHeader(nullptr)
    , m_recentList(nullptr)
    , m_favoriteHeader(nullptr)
    , m_favoriteList(nullptr)
    , m_categoryTree(nullptr)
{
    setupUI();
    applyDarkTheme();
    loadSettings();
    buildCategoryTree();
}

NodeToolbox::~NodeToolbox()
{
    saveSettings();
}

// -------------------------------------------------------------------
// UI 构建
// -------------------------------------------------------------------

void NodeToolbox::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // ---- 搜索框 ----
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(QStringLiteral("搜索节点..."));
    m_searchBox->setClearButtonEnabled(true);
    connect(m_searchBox, &QLineEdit::textChanged,
            this, &NodeToolbox::onSearchTextChanged);
    mainLayout->addWidget(m_searchBox);

    // ---- "最近使用" 可折叠分组（三角箭头）----
    m_recentHeader = new QPushButton(QStringLiteral("  ▼ 最近使用"), this);
    m_recentHeader->setFlat(true);
    m_recentHeader->setCursor(Qt::ArrowCursor);
    m_recentHeader->setStyleSheet("QPushButton { text-align: left; color: #c0c0c0; background: transparent; border: none; font-size: 11px; font-weight: bold; padding: 4px 2px; }");
    m_recentList = new QListWidget(this);
    m_recentList->setMaximumHeight(150);
    m_recentList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &NodeToolbox::onRecentItemDoubleClicked);
    connect(m_recentHeader, &QPushButton::clicked, this, [this]() {
        bool vis = !m_recentList->isVisible();
        m_recentList->setVisible(vis);
        m_recentHeader->setText(vis ? QStringLiteral("  ▼ 最近使用") : QStringLiteral("  ▶ 最近使用"));
    });
    mainLayout->addWidget(m_recentHeader);
    mainLayout->addWidget(m_recentList);

    // ---- "收藏夹" 可折叠分组（三角箭头，支持拖入）----
    m_favoriteHeader = new QPushButton(QStringLiteral("  ▼ 收藏夹"), this);
    m_favoriteHeader->setFlat(true);
    m_favoriteHeader->setCursor(Qt::ArrowCursor);
    m_favoriteHeader->setStyleSheet("QPushButton { text-align: left; color: #c0c0c0; background: transparent; border: none; font-size: 11px; font-weight: bold; padding: 4px 2px; }");
    m_favoriteList = new FavoriteDropListWidget(this, this);
    m_favoriteList->setMaximumHeight(150);
    m_favoriteList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_favoriteList, &QListWidget::itemDoubleClicked, this, &NodeToolbox::onFavoriteItemDoubleClicked);
    connect(m_favoriteHeader, &QPushButton::clicked, this, [this]() {
        bool vis = !m_favoriteList->isVisible();
        m_favoriteList->setVisible(vis);
        m_favoriteHeader->setText(vis ? QStringLiteral("  ▼ 收藏夹") : QStringLiteral("  ▶ 收藏夹"));
    });
    mainLayout->addWidget(m_favoriteHeader);
    mainLayout->addWidget(m_favoriteList);

    // ---- 分类树 ----
    m_categoryTree = new DragEnabledTreeWidget(this);
    m_categoryTree->setHeaderHidden(true);
    m_categoryTree->setIndentation(16);
    m_categoryTree->setAnimated(true);
    m_categoryTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_categoryTree, &QTreeWidget::itemDoubleClicked,
            this, &NodeToolbox::onTreeItemDoubleClicked);
    connect(m_categoryTree, &QTreeWidget::customContextMenuRequested,
            this, &NodeToolbox::onTreeContextMenu);

    mainLayout->addWidget(m_categoryTree, 1); // stretch=1: 占据剩余空间
}

// -------------------------------------------------------------------
// 暗色主题样式
// -------------------------------------------------------------------

void NodeToolbox::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "NodeToolbox {"
        "  background-color: #2b2b2b;"
        "  color: #e0e0e0;"
        "}"
        "QLineEdit {"
        "  background-color: #3a3a3a;"
        "  color: #e0e0e0;"
        "  border: 1px solid #4a4a4a;"
        "  border-radius: 3px;"
        "  padding: 3px 6px;"
        "  font-size: 12px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4EC9B0;"
        "}"
        "QGroupBox {"
        "  background-color: #2b2b2b;"
        "  color: #c0c0c0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 3px;"
        "  margin-top: 8px;"
        "  padding-top: 14px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  padding: 2px 6px;"
        "  color: #c0c0c0;"
        "}"
        "QGroupBox::indicator {"
        "  width: 12px;"
        "  height: 12px;"
        "}"
        "QListWidget {"
        "  background-color: #252525;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 2px;"
        "  font-size: 11px;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  padding: 2px 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #3a5a4a;"
        "  color: #e0e0e0;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #333333;"
        "}"
        "QTreeWidget {"
        "  background-color: #252525;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  border-radius: 2px;"
        "  font-size: 11px;"
        "  outline: none;"
        "}"
        "QTreeWidget::item {"
        "  padding: 2px 0px;"
        "}"
        "QTreeWidget::item:selected {"
        "  background-color: #3a5a4a;"
        "  color: #e0e0e0;"
        "}"
        "QTreeWidget::item:hover {"
        "  background-color: #333333;"
        "}"
        "QTreeWidget::branch:has-children:!has-siblings:closed,"
        "QTreeWidget::branch:closed:has-children:has-siblings {"
        "  border-image: none;"
        "}"
        "QTreeWidget::branch:open:has-children:!has-siblings,"
        "QTreeWidget::branch:open:has-children:has-siblings {"
        "  border-image: none;"
        "}"
    ));
}

// -------------------------------------------------------------------
// 构建分类树
// -------------------------------------------------------------------

void NodeToolbox::buildCategoryTree()
{
    m_categoryTree->clear();
    m_colorCache.clear();

    const auto registrations = NodeFactory::instance().allRegistrations();

    // categoryKey -> tree item 的映射，避免重复创建相同分类项
    QHash<QString, QTreeWidgetItem *> level1Items;
    QHash<QString, QTreeWidgetItem *> level2Items; // key = "L1/L2"

    for (const auto &reg : registrations) {
        // 获取输出颜色（getNodeOutputColor 内部自动缓存）
        QColor color = getNodeOutputColor(reg.typeName);
        QPixmap icon = createColorIcon(color);

        // 解析 categoryPath（以 "/" 分隔）
        QStringList parts = reg.categoryPath.split(QStringLiteral("/"),
                                                   Qt::SkipEmptyParts);

        QTreeWidgetItem *parent = nullptr;

        if (parts.isEmpty()) {
            // 没有分类路径，直接放在根级别（罕见情况）
            auto *leaf = new QTreeWidgetItem(m_categoryTree);
            leaf->setText(0, reg.displayName);
            leaf->setData(0, Qt::UserRole, reg.typeName);
            leaf->setIcon(0, QIcon(icon));
            leaf->setFlags(leaf->flags() | Qt::ItemIsDragEnabled);
            continue;
        }

        // Level 1: 主分类
        const QString &l1 = parts[0];
        if (!level1Items.contains(l1)) {
            auto *item = new QTreeWidgetItem(m_categoryTree);
            item->setText(0, l1);
            item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
            // 默认折叠
            item->setExpanded(false);
            level1Items[l1] = item;
        }
        parent = level1Items[l1];

        if (parts.size() >= 2) {
            // Level 2: 子分类
            QString l2Key = l1 + QStringLiteral("/") + parts[1];
            if (!level2Items.contains(l2Key)) {
                auto *item = new QTreeWidgetItem(parent);
                item->setText(0, parts[1]);
                item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
                item->setExpanded(false);
                level2Items[l2Key] = item;
            }
            parent = level2Items[l2Key];
        }

        // 处理第三级及以上子分类
        if (parts.size() >= 3) {
            for (int i = 2; i < parts.size(); ++i) {
                // 查找或创建此子分类
                QString subKey = reg.categoryPath.section('/', 0, i);
                bool found = false;
                for (int j = 0; j < parent->childCount(); ++j) {
                    if (parent->child(j)->text(0) == parts[i]) {
                        parent = parent->child(j);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    auto *sub = new QTreeWidgetItem(parent);
                    sub->setText(0, parts[i]);
                    sub->setFlags(sub->flags() & ~Qt::ItemIsDragEnabled);
                    sub->setExpanded(false);
                    parent = sub;
                }
            }
        }

        // 叶子节点（挂在最后一级分类或直接二级分类下）
        auto *leaf = new QTreeWidgetItem(parent);
        leaf->setText(0, reg.displayName);
        leaf->setData(0, Qt::UserRole, reg.typeName);
        leaf->setIcon(0, QIcon(icon));
        leaf->setFlags(leaf->flags() | Qt::ItemIsDragEnabled);
    }

    // 确保所有一级分类默认折叠
    for (auto it = level1Items.begin(); it != level1Items.end(); ++it) {
        it.value()->setExpanded(false);
    }
}

// -------------------------------------------------------------------
// QSettings 持久化
// -------------------------------------------------------------------

void NodeToolbox::loadSettings()
{
    QSettings settings(QStringLiteral("SciFlow"), QStringLiteral("NodeToolbox"));

    m_recentTypes = settings.value(QStringLiteral("Recent"),
                                   QStringList()).toStringList();
    m_favoriteTypes = settings.value(QStringLiteral("Favorites"),
                                     QStringList()).toStringList();

    // 确保不超过上限
    while (m_recentTypes.size() > MaxRecent)
        m_recentTypes.removeLast();

    // 填充最近使用列表
    m_recentList->clear();
    for (const QString &typeName : m_recentTypes) {
        QString display = displayNameForType(typeName);
        auto *item = new QListWidgetItem(display, m_recentList);
        item->setData(Qt::UserRole, typeName);
    }

    // 填充收藏列表
    m_favoriteList->clear();
    for (const QString &typeName : m_favoriteTypes) {
        QString display = displayNameForType(typeName);
        auto *item = new QListWidgetItem(display, m_favoriteList);
        item->setData(Qt::UserRole, typeName);
    }
}

void NodeToolbox::saveSettings()
{
    QSettings settings(QStringLiteral("SciFlow"), QStringLiteral("NodeToolbox"));
    settings.setValue(QStringLiteral("Recent"), m_recentTypes);
    settings.setValue(QStringLiteral("Favorites"), m_favoriteTypes);
}

// -------------------------------------------------------------------
// 最近使用
// -------------------------------------------------------------------

void NodeToolbox::addToRecent(const QString &typeName)
{
    if (typeName.isEmpty())
        return;

    // 去重：如果已存在，先移除
    m_recentTypes.removeAll(typeName);
    // 置顶
    m_recentTypes.prepend(typeName);
    // 保留最近10个
    m_recentTypes = m_recentTypes.mid(0, 10);

    // 刷新 UI
    m_recentList->clear();
    for (const QString &tn : m_recentTypes) {
        QString display = displayNameForType(tn);
        auto *item = new QListWidgetItem(display, m_recentList);
        item->setData(Qt::UserRole, tn);
    }

    saveSettings();
}

// -------------------------------------------------------------------
// 收藏
// -------------------------------------------------------------------

void NodeToolbox::addToFavorite(const QString &typeName)
{
    if (typeName.isEmpty() || m_favoriteTypes.contains(typeName))
        return;

    m_favoriteTypes.append(typeName);

    QString display = displayNameForType(typeName);
    auto *item = new QListWidgetItem(display, m_favoriteList);
    item->setData(Qt::UserRole, typeName);

    saveSettings();
}

void NodeToolbox::removeFromFavorite(const QString &typeName)
{
    if (typeName.isEmpty())
        return;

    m_favoriteTypes.removeAll(typeName);

    // 从列表中移除对应项
    for (int i = 0; i < m_favoriteList->count(); ++i) {
        auto *item = m_favoriteList->item(i);
        if (item->data(Qt::UserRole).toString() == typeName) {
            delete m_favoriteList->takeItem(i);
            break;
        }
    }

    saveSettings();
}

// -------------------------------------------------------------------
// 搜索过滤
// -------------------------------------------------------------------

// 前置声明：递归过滤树节点的辅助函数
static bool filterTreeRecursive(QTreeWidgetItem *item, const QString &filter);

void NodeToolbox::onSearchTextChanged(const QString &text)
{
    filterTree(text.trimmed());
}

void NodeToolbox::filterTree(const QString &filter)
{
    // 遍历所有顶层节点，递归处理
    for (int i = 0; i < m_categoryTree->topLevelItemCount(); ++i) {
        filterTreeRecursive(m_categoryTree->topLevelItem(i), filter);
    }
}

/// 递归过滤树节点；返回 true 表示该节点（或其子孙）可见
static bool filterTreeRecursive(QTreeWidgetItem *item, const QString &filter)
{
    if (!item)
        return false;

    bool hasVisibleChild = false;

    if (item->childCount() == 0) {
        // 叶子节点：检查文本是否匹配
        QString displayText = item->text(0);
        QString typeName = item->data(0, Qt::UserRole).toString();

        bool match = false;
        if (filter.isEmpty()) {
            match = true;
        } else {
            QRegularExpression regex(QRegularExpression::escape(filter),
                                     QRegularExpression::CaseInsensitiveOption);
            match = regex.match(displayText).hasMatch()
                 || regex.match(typeName).hasMatch();
        }

        item->setHidden(!match);
        return match;
    } else {
        // 中间节点：递归检查子节点
        for (int i = 0; i < item->childCount(); ++i) {
            if (filterTreeRecursive(item->child(i), filter))
                hasVisibleChild = true;
        }

        item->setHidden(!hasVisibleChild);

        // 当有搜索词且有可见子节点时，自动展开
        if (!filter.isEmpty() && hasVisibleChild)
            item->setExpanded(true);
        else if (filter.isEmpty())
            item->setExpanded(false); // 清空搜索时恢复折叠状态

        return hasVisibleChild;
    }
}

// -------------------------------------------------------------------
// 信号槽：交互事件
// -------------------------------------------------------------------

void NodeToolbox::onTreeItemDoubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (!item || item->childCount() > 0)
        return; // 忽略非叶子节点

    QString typeName = item->data(0, Qt::UserRole).toString();
    if (!typeName.isEmpty()) {
        addToRecent(typeName);
        emit nodeDoubleClicked(typeName);
    }
}

void NodeToolbox::onRecentItemDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    QString typeName = item->data(Qt::UserRole).toString();
    if (!typeName.isEmpty()) {
        addToRecent(typeName); // 点击后再次置顶
        emit nodeDoubleClicked(typeName);
    }
}

void NodeToolbox::onFavoriteItemDoubleClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    QString typeName = item->data(Qt::UserRole).toString();
    if (!typeName.isEmpty()) {
        addToRecent(typeName);
        emit nodeDoubleClicked(typeName);
    }
}

void NodeToolbox::onTreeContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_categoryTree->itemAt(pos);
    if (!item || item->childCount() > 0)
        return; // 只对叶子节点弹出菜单

    QString typeName = item->data(0, Qt::UserRole).toString();
    if (typeName.isEmpty())
        return;

    bool isFav = m_favoriteTypes.contains(typeName);

    QMenu menu(this);
    menu.setStyleSheet(QStringLiteral(
        "QMenu {"
        "  background-color: #2b2b2b;"
        "  color: #e0e0e0;"
        "  border: 1px solid #4a4a4a;"
        "  padding: 2px;"
        "}"
        "QMenu::item {"
        "  padding: 4px 20px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #3a5a4a;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background: #4a4a4a;"
        "  margin: 2px 5px;"
        "}"
    ));

    if (isFav) {
        QAction *act = menu.addAction(QStringLiteral("取消收藏"));
        connect(act, &QAction::triggered, this, [this, typeName]() {
            removeFromFavorite(typeName);
        });
    } else {
        QAction *act = menu.addAction(QStringLiteral("加入收藏"));
        connect(act, &QAction::triggered, this, [this, typeName]() {
            addToFavorite(typeName);
        });
    }

    menu.exec(m_categoryTree->viewport()->mapToGlobal(pos));
}

// -------------------------------------------------------------------
// 辅助方法
// -------------------------------------------------------------------

QColor NodeToolbox::getNodeOutputColor(const QString &typeName) const
{
    // 先查缓存
    if (m_colorCache.contains(typeName))
        return m_colorCache[typeName];

    QColor color(QStringLiteral("#666666")); // 默认灰色

    // 尝试通过工厂临时创建节点来检查其输出数据类型
    auto tempNode = NodeFactory::instance().createNode(typeName);
    if (tempNode) {
        const auto &ports = tempNode->outputPorts();
        if (!ports.isEmpty()) {
            color = QColor(dataTypeColor(ports.first().dataType));
        }
    }

    // 缓存结果（包括失败情况，避免重复创建临时节点）
    m_colorCache[typeName] = color;
    return color;
}

QPixmap NodeToolbox::createColorIcon(const QColor &color, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(1, 1, size - 2, size - 2, 2, 2);
    painter.end();

    return pix;
}

QString NodeToolbox::displayNameForType(const QString &typeName) const
{
    const auto registrations = NodeFactory::instance().allRegistrations();
    for (const auto &reg : registrations) {
        if (reg.typeName == typeName)
            return reg.displayName;
    }
    return typeName; // fallback: 直接返回 typeName
}
