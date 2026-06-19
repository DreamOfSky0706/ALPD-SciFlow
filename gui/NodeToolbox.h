#ifndef SCIFLOW_NODETOOLBOX_H
#define SCIFLOW_NODETOOLBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QTreeWidget>
#include <QListWidget>
#include <QPushButton>
#include <QColor>
#include <QPixmap>

class NodeToolbox : public QWidget
{
    Q_OBJECT

public:
    explicit NodeToolbox(QWidget *parent = nullptr);
    ~NodeToolbox() override;

    /// 记录最近使用的节点类型（最多保留 10 个，去重并置顶）
    void addToRecent(const QString &typeName);

    /// 添加节点类型到收藏夹
    void addToFavorite(const QString &typeName);

    /// 从收藏夹移除节点类型
    void removeFromFavorite(const QString &typeName);

signals:
    /// 双击叶子节点时发射，携带 typeName
    void nodeDoubleClicked(const QString &typeName);

private slots:
    void onSearchTextChanged(const QString &text);
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onTreeContextMenu(const QPoint &pos);
    void onRecentItemDoubleClicked(QListWidgetItem *item);
    void onFavoriteItemDoubleClicked(QListWidgetItem *item);

private:
    void setupUI();
    void applyDarkTheme();
    void buildCategoryTree();
    void loadSettings();
    void saveSettings();
    void filterTree(const QString &filter);

    /// 通过临时创建节点实例来获取其主输出数据类型的颜色
    QColor getNodeOutputColor(const QString &typeName) const;

    /// 创建纯色方块图标
    static QPixmap createColorIcon(const QColor &color, int size = 12);

    /// 根据 displayName 查找 typeName（用于最近/收藏列表的友好显示）
    QString displayNameForType(const QString &typeName) const;

    QLineEdit   *m_searchBox;
    QPushButton *m_recentHeader;
    QListWidget *m_recentList;
    QPushButton *m_favoriteHeader;
    QListWidget *m_favoriteList;
    QTreeWidget *m_categoryTree;

    QStringList  m_recentTypes;
    QStringList  m_favoriteTypes;

    /// 缓存 typeName → 输出颜色，避免重复创建临时节点
    mutable QHash<QString, QColor> m_colorCache;

    static const int MaxRecent = 10;

public:
    static const QString MimeType;
};

#endif // SCIFLOW_NODETOOLBOX_H
