#include "PreviewPanel.h"

#include "../common/NodeData.h"
#include "../common/DataTable.h"
#include "../common/DataType.h"
#include "../common/Utility.h"

#include <opencv2/core.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QScrollArea>
#include <QTableView>
#include <QHeaderView>
#include <QTextEdit>
#include <QFont>
#include <QDialog>
#include <QScrollBar>
#include <QPixmap>
#include <QImage>
#include <QResizeEvent>
#include <QAbstractTableModel>
#include <QMouseEvent>

// ===================================================================
// 内部辅助类
// ===================================================================

// -------------------------------------------------------------------
// ScaledImageLabel — 自动缩放图像以适应控件大小，保持宽高比
// -------------------------------------------------------------------
class ScaledImageLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ScaledImageLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);
        setMinimumSize(40, 40);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setOriginalPixmap(const QPixmap &pix)
    {
        m_original = pix;
        updateScaledPixmap();
    }

    const QPixmap &originalPixmap() const { return m_original; }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QLabel::resizeEvent(event);
        updateScaledPixmap();
    }

private:
    void updateScaledPixmap()
    {
        if (m_original.isNull())
            return;

        QSize targetSize = size() - QSize(4, 4); // 留一点边距
        if (targetSize.width() <= 0 || targetSize.height() <= 0)
            return;

        QPixmap scaled = m_original.scaled(targetSize,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation);
        setPixmap(scaled);
    }

    QPixmap m_original;
};

// -------------------------------------------------------------------
// ImagePreviewWidget — 图像预览（缩略图 + 信息，双击全尺寸）
// -------------------------------------------------------------------
class ImagePreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImagePreviewWidget(const QImage &image, QWidget *parent = nullptr)
        : QWidget(parent), m_image(image)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setSpacing(4);

        // 可缩放图像
        m_imageLabel = new ScaledImageLabel(this);
        m_imageLabel->setOriginalPixmap(QPixmap::fromImage(m_image));
        m_imageLabel->setCursor(Qt::PointingHandCursor);
        m_imageLabel->installEventFilter(this);
        layout->addWidget(m_imageLabel, 1);

        // 信息标签
        int channels = 3;
        switch (m_image.format()) {
        case QImage::Format_Grayscale8:
        case QImage::Format_Grayscale16:
            channels = 1;
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        case QImage::Format_RGBA8888:
        case QImage::Format_RGBA8888_Premultiplied:
            channels = 4;
            break;
        default:
            channels = m_image.hasAlphaChannel() ? 4 : 3;
            break;
        }

        m_infoLabel = new QLabel(this);
        m_infoLabel->setAlignment(Qt::AlignCenter);
        m_infoLabel->setStyleSheet(QStringLiteral("color: #a0a0a0; font-size: 11px;"));
        m_infoLabel->setText(
            QStringLiteral("%1x%2, %3 channels")
                .arg(m_image.width())
                .arg(m_image.height())
                .arg(channels));
        layout->addWidget(m_infoLabel);
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (obj == m_imageLabel && event->type() == QEvent::MouseButtonDblClick) {
            showFullSizeDialog();
            return true;
        }
        return QWidget::eventFilter(obj, event);
    }

private:
    void showFullSizeDialog()
    {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle(QStringLiteral("图像预览"));
        dialog->setMinimumSize(400, 300);
        dialog->resize(
            qMin(m_image.width() + 40, 1200),
            qMin(m_image.height() + 80, 900));
        dialog->setStyleSheet(QStringLiteral(
            "QDialog { background-color: #1e1e1e; }"));

        auto *dlgLayout = new QVBoxLayout(dialog);
        dlgLayout->setContentsMargins(0, 0, 0, 0);

        auto *scrollArea = new QScrollArea(dialog);
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollArea->setStyleSheet(QStringLiteral(
            "QScrollArea { background-color: #1e1e1e; border: none; }"));

        auto *fullLabel = new QLabel;
        fullLabel->setPixmap(QPixmap::fromImage(m_image));
        fullLabel->setAlignment(Qt::AlignCenter);
        scrollArea->setWidget(fullLabel);

        dlgLayout->addWidget(scrollArea);
        dialog->exec();

        delete dialog;
    }

    ScaledImageLabel *m_imageLabel;
    QLabel           *m_infoLabel;
    QImage            m_image;
};

// -------------------------------------------------------------------
// DataTableModel — 将 DataTable 包装为 QAbstractTableModel（最多 50 行）
// -------------------------------------------------------------------
class DataTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DataTableModel(const DataTable &table, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_table(table) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return qMin(m_table.rowCount(), 50); // 最多展示 50 行
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return m_table.columnCount();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        if (role == Qt::DisplayRole) {
            return m_table.value(index.row(), index.column());
        }

        if (role == Qt::TextAlignmentRole) {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            if (section >= 0 && section < m_table.columnCount())
                return m_table.columnNames().at(section);
            return QString::number(section + 1);
        }

        // 垂直表头：行号
        return QString::number(section + 1);
    }

private:
    DataTable m_table;
};

// ===================================================================
// PreviewPanel 实现
// ===================================================================

PreviewPanel::PreviewPanel(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(nullptr)
    , m_emptyLabel(nullptr)
{
    setupUI();
    applyDarkTheme();
}

PreviewPanel::~PreviewPanel() = default;

// -------------------------------------------------------------------
// UI 构建
// -------------------------------------------------------------------

void PreviewPanel::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    // Tab 页容器（多输出端口时使用）
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setVisible(false);
    mainLayout->addWidget(m_tabWidget, 1);

    // 空状态标签
    m_emptyLabel = new QLabel(QStringLiteral("此节点无输出数据"), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral(
        "color: #808080; font-size: 13px;"));
    m_emptyLabel->setVisible(true);
    mainLayout->addWidget(m_emptyLabel, 1);
}

// -------------------------------------------------------------------
// 暗色主题
// -------------------------------------------------------------------

void PreviewPanel::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "PreviewPanel {"
        "  background-color: #2b2b2b;"
        "  color: #e0e0e0;"
        "}"
        "QTabWidget::pane {"
        "  background-color: #252525;"
        "  border: 1px solid #3a3a3a;"
        "  border-top: none;"
        "}"
        "QTabBar::tab {"
        "  background-color: #2b2b2b;"
        "  color: #a0a0a0;"
        "  border: 1px solid #3a3a3a;"
        "  border-bottom: none;"
        "  padding: 4px 12px;"
        "  font-size: 11px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: #252525;"
        "  color: #e0e0e0;"
        "  border-bottom: 2px solid #4EC9B0;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: #333333;"
        "  color: #c0c0c0;"
        "}"
        "QTableView {"
        "  background-color: #1e1e1e;"
        "  color: #e0e0e0;"
        "  gridline-color: #3a3a3a;"
        "  border: 1px solid #3a3a3a;"
        "  selection-background-color: #3a5a4a;"
        "  font-size: 11px;"
        "}"
        "QTableView::item {"
        "  padding: 2px 6px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #333333;"
        "  color: #c0c0c0;"
        "  border: 1px solid #3a3a3a;"
        "  padding: 3px 6px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "}"
        "QTextEdit {"
        "  background-color: #1e1e1e;"
        "  color: #e0e0e0;"
        "  border: 1px solid #3a3a3a;"
        "  selection-background-color: #3a5a4a;"
        "  font-size: 12px;"
        "}"
        "QLabel {"
        "  color: #e0e0e0;"
        "  background: transparent;"
        "}"
        "QScrollArea {"
        "  background-color: #1e1e1e;"
        "  border: 1px solid #3a3a3a;"
        "}"
        "QScrollBar:vertical {"
        "  background: #2b2b2b;"
        "  width: 8px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #4a4a4a;"
        "  border-radius: 4px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "  background: #2b2b2b;"
        "  height: 8px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #4a4a4a;"
        "  border-radius: 4px;"
        "  min-width: 20px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "  width: 0px;"
        "}"
    ));
}

// -------------------------------------------------------------------
// 预览节点
// -------------------------------------------------------------------

void PreviewPanel::previewNode(NodeBase *node)
{
    clearPreview();

    if (!node)
        return;

    const auto &ports = node->outputPorts();

    // 统计有效输出
    struct PortData {
        QString  name;
        std::shared_ptr<NodeData> data;
    };
    QList<PortData> validPorts;

    for (const auto &port : ports) {
        auto data = node->getOutput(port.name);
        if (data && !data->isNull()) {
            validPorts.append({port.name, data});
        }
    }

    if (validPorts.isEmpty()) {
        // 无有效输出，显示空状态
        m_emptyLabel->setVisible(true);
        m_tabWidget->setVisible(false);
        return;
    }

    m_emptyLabel->setVisible(false);
    m_tabWidget->setVisible(true);

    // 为每个有效输出端口创建一个标签页
    for (const auto &pd : validPorts) {
        QWidget *preview = createPreviewForData(pd.data);
        if (preview) {
            m_tabWidget->addTab(preview, pd.name);
        }
    }

    // 如果没有成功创建任何预览（所有数据类型未知），回退到空状态
    if (m_tabWidget->count() == 0) {
        m_tabWidget->setVisible(false);
        m_emptyLabel->setVisible(true);
    }
}

void PreviewPanel::clearPreview()
{
    // 移除所有标签页（不删除 widget，Qt 父对象机制会自动清理）
    while (m_tabWidget->count() > 0) {
        QWidget *w = m_tabWidget->widget(0);
        m_tabWidget->removeTab(0);
        delete w;
    }

    m_tabWidget->setVisible(false);
    m_emptyLabel->setVisible(true);
}

// -------------------------------------------------------------------
// 根据数据类型分发预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createPreviewForData(const std::shared_ptr<NodeData>& data)
{
    if (!data || data->isNull())
        return nullptr;

    switch (data->dataType()) {
    case DataType::Image:
        return createImagePreview(data);
    case DataType::ImageList:
        return createImageListPreview(data);
    case DataType::DataTable:
        return createDataTablePreview(data);
    case DataType::Numeric:
        return createNumericPreview(data);
    case DataType::Text:
        return createTextPreview(data);
    default:
        return nullptr;
    }
}

// -------------------------------------------------------------------
// Image 预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createImagePreview(const std::shared_ptr<NodeData>& data)
{
    QImage image;

    // 从 cv::Mat 转换
    cv::Mat mat = data->toImage();
    if (!mat.empty()) {
        image = Utility::matToQImage(mat);
    }

    if (image.isNull()) {
        auto *errorLabel = new QLabel(QStringLiteral("无法解析图像数据"));
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet(QStringLiteral("color: #e04040; font-size: 12px;"));
        return errorLabel;
    }

    return new ImagePreviewWidget(image);
}

// -------------------------------------------------------------------
// ImageList 预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createImageListPreview(const std::shared_ptr<NodeData>& data)
{
    std::vector<cv::Mat> mats = data->toImageList();
    QList<QImage> images;
    for (const auto& mat : mats) {
        if (!mat.empty())
            images.append(Utility::matToQImage(mat));
    }

    if (images.isEmpty()) {
        auto *errorLabel = new QLabel(QStringLiteral("图像列表为空"));
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet(QStringLiteral("color: #808080; font-size: 12px;"));
        return errorLabel;
    }

    // 滚动区域
    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 内部容器
    auto *container = new QWidget;
    auto *containerLayout = new QHBoxLayout(container);
    containerLayout->setContentsMargins(6, 6, 6, 6);
    containerLayout->setSpacing(8);
    containerLayout->addStretch(); // 推送缩略图到左侧

    int index = 0;
    for (const QImage &img : images) {
        auto *thumbWidget = new QWidget;
        auto *thumbLayout = new QVBoxLayout(thumbWidget);
        thumbLayout->setContentsMargins(2, 2, 2, 2);
        thumbLayout->setSpacing(2);

        // 缩略图（100px 宽，保持比例）
        auto *thumbLabel = new QLabel;
        thumbLabel->setFixedWidth(100);
        thumbLabel->setAlignment(Qt::AlignCenter);
        thumbLabel->setStyleSheet(QStringLiteral(
            "background-color: #1a1a1a; border: 1px solid #3a3a3a;"));

        QPixmap thumb = QPixmap::fromImage(img).scaledToWidth(
            100, Qt::SmoothTransformation);
        thumbLabel->setPixmap(thumb);
        thumbLabel->setFixedHeight(thumb.height() + 4);
        thumbLayout->addWidget(thumbLabel);

        // 索引标签
        auto *indexLabel = new QLabel(QStringLiteral("#%1").arg(index));
        indexLabel->setAlignment(Qt::AlignCenter);
        indexLabel->setStyleSheet(QStringLiteral(
            "color: #a0a0a0; font-size: 10px; background: transparent;"));
        thumbLayout->addWidget(indexLabel);

        containerLayout->addWidget(thumbWidget);
        ++index;
    }

    containerLayout->addStretch();
    scrollArea->setWidget(container);

    return scrollArea;
}

// -------------------------------------------------------------------
// DataTable 预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createDataTablePreview(const std::shared_ptr<NodeData>& data)
{
    auto tablePtr = data->toDataTable();
    if (!tablePtr) return nullptr;
    const DataTable& table = *tablePtr;

    if (table.columnCount() == 0) {
        auto *errorLabel = new QLabel(QStringLiteral("数据表为空"));
        errorLabel->setAlignment(Qt::AlignCenter);
        errorLabel->setStyleSheet(QStringLiteral("color: #808080; font-size: 12px;"));
        return errorLabel;
    }

    auto *tableView = new QTableView;
    tableView->setAlternatingRowColors(true);
    tableView->setStyleSheet(QStringLiteral(
        "QTableView { alternate-background-color: #222222; }"));

    auto *model = new DataTableModel(table, tableView);
    // 模型由 tableView 通过 setModel 管理生命周期
    // 但我们需要确保它在 tableView 存活期间存在
    tableView->setModel(model);

    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(true);
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setShowGrid(true);

    // 自适应列宽
    tableView->resizeColumnsToContents();

    return tableView;
}

// -------------------------------------------------------------------
// Numeric 预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createNumericPreview(const std::shared_ptr<NodeData>& data)
{
    double value = data->toNumeric();

    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignCenter);

    auto *valueLabel = new QLabel;
    QFont bigFont = valueLabel->font();
    bigFont.setPointSize(24);
    bigFont.setBold(true);
    valueLabel->setFont(bigFont);
    valueLabel->setAlignment(Qt::AlignCenter);

    // 对整数不显示多余的小数点
    if (qFuzzyCompare(value, qRound(value))) {
        valueLabel->setText(QString::number(qRound(value)));
    } else {
        valueLabel->setText(QString::number(value, 'g', 12));
    }

    layout->addWidget(valueLabel);

    return container;
}

// -------------------------------------------------------------------
// Text 预览
// -------------------------------------------------------------------

QWidget *PreviewPanel::createTextPreview(const std::shared_ptr<NodeData>& data)
{
    QString text = data->toText();

    auto *textEdit = new QTextEdit;
    textEdit->setReadOnly(true);
    textEdit->setUndoRedoEnabled(false);

    QFont monoFont(QStringLiteral("Consolas"), 10);
    monoFont.setStyleHint(QFont::Monospace);
    textEdit->setFont(monoFont);

    textEdit->setPlainText(text);

    return textEdit;
}

// -------------------------------------------------------------------
// 必须包含 ScaledImageLabel 的 .moc 以确保 Q_OBJECT 宏正确处理
// （在 CMake 项目中由 AUTOMOC 自动处理，此处显式包含为备选方案）
// -------------------------------------------------------------------
#include "PreviewPanel.moc"
