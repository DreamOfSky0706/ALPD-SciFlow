// nodes/layout/MultiImageLayout.cpp
#include "MultiImageLayout.h"
#include "Utility.h"
#include <opencv2/imgproc.hpp>
#include <QSet>

void MultiImageLayout::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("rows", "行数", ParamType::Combo, QString("2"),
			 { {"options", QStringList{"1", "2", "3", "4"}} });
	addParam("cols", "列数", ParamType::Combo, QString("2"),
			 { {"options", QStringList{"1", "2", "3", "4"}} });

	// 网格配置通过GridEditor控件编辑
	// cells: [{r, c, enabled, group}...]
	QVariantList defaultCells;
	for (int r = 0; r < 2; ++r)
		for (int c = 0; c < 2; ++c)
		{
			QVariantMap cell;
			cell["r"] = r; cell["c"] = c;
			cell["enabled"] = true;
			cell["group"] = r * 2 + c;
			defaultCells << cell;
		}
	QVariantMap defaultGrid;
	defaultGrid["rows"] = 2; defaultGrid["cols"] = 2;
	defaultGrid["cells"] = defaultCells;
	addParam("grid_config", "网格配置", ParamType::GridEditor, defaultGrid);

	addParam("size_mode", "尺寸模式", ParamType::Combo, QString("像素"),
			 { {"options", QStringList{"像素", "物理尺寸(mm)+DPI", "物理尺寸(inch)+DPI"}} });
	addParam("canvas_width", "画布宽度", ParamType::Int, 1600, { {"min", 100}, {"max", 20000} });
	addParam("canvas_height", "画布高度", ParamType::Int, 1200, { {"min", 100}, {"max", 20000} });
	addParam("dpi", "DPI", ParamType::Int, 300,
			 { {"min", 72}, {"max", 2400}, {"visible_when", "size_mode!=像素"} });
	addParam("gap", "格间距", ParamType::IntSlider, 10, { {"min", 0}, {"max", 100} });
	addParam("outer_margin", "外边距", ParamType::IntSlider, 20, { {"min", 0}, {"max", 100} });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("resize_mode", "缩放模式", ParamType::Combo, QString("等比缩放适配"),
			 { {"options", QStringList{"等比缩放适配", "拉伸填满", "不缩放居中"}} });
	addParam("show_labels", "显示编号", ParamType::Bool, true);
	addParam("label_style", "编号样式", ParamType::Combo, QString("(a)(b)(c)..."),
			 { {"options", QStringList{"(a)(b)(c)...", "(A)(B)(C)...", "(1)(2)(3)...", "(i)(ii)(iii)..."}} });
	addParam("label_font_size", "编号字号", ParamType::IntSlider, 14, { {"min", 8}, {"max", 36} });
	addParam("label_color", "编号颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });

	// 初始创建4个输入端口
	rebuildPorts();
}

void MultiImageLayout::onParamChanged(const QString& paramName)
{
	NodeBase::onParamChanged(paramName);
	if (paramName == "grid_config" || paramName == "rows" || paramName == "cols")
	{
		rebuildPorts();
	}
}

int MultiImageLayout::countEnabledGroups() const
{
	QVariantMap gridConfig = param("grid_config").toMap();
	QVariantList cells = gridConfig.value("cells").toList();

	QSet<int> groups;
	for (const auto& c : cells)
	{
		QVariantMap cell = c.toMap();
		if (cell.value("enabled", true).toBool())
		{
			groups.insert(cell.value("group", -1).toInt());
		}
	}
	return groups.size();
}

void MultiImageLayout::rebuildPorts()
{
	clearInputPorts();
	int numInputs = countEnabledGroups();
	if (numInputs < 1) numInputs = 1;

	for (int i = 0; i < numInputs; ++i)
	{
		addInputPort(QString("input_%1").arg(i), DataType::Image, false);
	}
}

void MultiImageLayout::process()
{
	QVariantMap gridConfig = param("grid_config").toMap();
	QVariantList cells = gridConfig.value("cells").toList();

	QString sizeMode = param("size_mode").toString();
	int canvasW = param("canvas_width").toInt();
	int canvasH = param("canvas_height").toInt();
	int dpi = param("dpi").toInt();

	if (sizeMode == "物理尺寸(mm)+DPI")
	{
		canvasW = safeRound((canvasW / 25.4 * dpi));
		canvasH = safeRound((canvasH / 25.4 * dpi));
	}
	else if (sizeMode == "物理尺寸(inch)+DPI")
	{
		canvasW = safeRound((canvasW * dpi));
		canvasH = safeRound((canvasH * dpi));
	}

	int gap = param("gap").toInt();
	int margin = param("outer_margin").toInt();
	QColor bgColor = Utility::arrayToColor(param("background_color").toList());
	QString resizeMode = param("resize_mode").toString();
	bool showLabels = param("show_labels").toBool();

	int gridRows = param("rows").toString().toInt();
	int gridCols = param("cols").toString().toInt();

	cv::Scalar bgScalar = Utility::colorToScalar(bgColor, false);
	cv::Mat canvas(canvasH, canvasW, CV_8UC3, bgScalar);

	// 可用区域
	int availW = canvasW - margin * 2 - (gridCols - 1) * gap;
	int availH = canvasH - margin * 2 - (gridRows - 1) * gap;
	int cellW = availW / gridCols;
	int cellH = availH / gridRows;

	// 收集启用的组及其占据的区域
	QMap<int, QRect> groupRects; // group -> 合并后的像素区域
	QSet<int> processedGroups;

	for (const auto& c : cells)
	{
		QVariantMap cell = c.toMap();
		if (!cell.value("enabled", true).toBool()) continue;
		int group = cell.value("group", -1).toInt();
		int r = cell.value("r", 0).toInt();
		int col = cell.value("c", 0).toInt();

		int x = margin + col * (cellW + gap);
		int y = margin + r * (cellH + gap);
		QRect cellRect(x, y, cellW, cellH);

		if (groupRects.contains(group))
		{
			groupRects[group] = groupRects[group].united(cellRect);
		}
		else
		{
			groupRects[group] = cellRect;
		}
	}

	// 按组顺序分配输入端口
	QList<int> sortedGroups = groupRects.keys();
	std::sort(sortedGroups.begin(), sortedGroups.end());

	// 标签生成
	auto makeLabel = [&](int idx) -> QString
		{
			QString style = param("label_style").toString();
			if (style.startsWith("(A)"))
				return QString("(%1)").arg(QChar('A' + idx));
			if (style.startsWith("(1)"))
				return QString("(%1)").arg(idx + 1);
			if (style.startsWith("(i)"))
			{
				QStringList roman = { "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix", "x",
									 "xi", "xii", "xiii", "xiv", "xv", "xvi" };
				return QString("(%1)").arg(idx < roman.size() ? roman[idx] : QString::number(idx + 1));
			}
			return QString("(%1)").arg(QChar('a' + idx));
		};

	QImage qCanvas = Utility::matToQImage(canvas);
	if (qCanvas.format() != QImage::Format_RGB888)
		qCanvas = qCanvas.convertToFormat(QImage::Format_RGB888);
	QPainter painter(&qCanvas);
	painter.setRenderHint(QPainter::Antialiasing);

	for (int i = 0; i < sortedGroups.size(); ++i)
	{
		QString portName = QString("input_%1").arg(i);
		auto imgData = getInput(portName);

		QRect rect = groupRects[sortedGroups[i]];

		if (imgData && !imgData->isNull())
		{
			cv::Mat img = imgData->toImage();
			if (img.channels() == 1) cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
			if (img.channels() == 4) cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);

			cv::Mat cell;
			if (resizeMode == "拉伸填满")
			{
				cv::resize(img, cell, cv::Size(rect.width(), rect.height()));
			}
			else if (resizeMode == "不缩放居中")
			{
				cell = cv::Mat(rect.height(), rect.width(), CV_8UC3, bgScalar);
				int ox = (rect.width() - img.cols) / 2;
				int oy = (rect.height() - img.rows) / 2;
				int sx = std::max(0, -ox), sy = std::max(0, -oy);
				int dx = std::max(0, ox), dy = std::max(0, oy);
				int cw = std::min(img.cols - sx, rect.width() - dx);
				int ch = std::min(img.rows - sy, rect.height() - dy);
				if (cw > 0 && ch > 0)
					img(cv::Rect(sx, sy, cw, ch)).copyTo(cell(cv::Rect(dx, dy, cw, ch)));
			}
			else
			{
				double s = std::min(static_cast<double>(rect.width()) / img.cols,
									static_cast<double>(rect.height()) / img.rows);
				int nw = safeRound((img.cols * s));
				int nh = safeRound((img.rows * s));
				cv::Mat resized;
				cv::resize(img, resized, cv::Size(nw, nh));
				cell = cv::Mat(rect.height(), rect.width(), CV_8UC3, bgScalar);
				int ox = (rect.width() - nw) / 2;
				int oy = (rect.height() - nh) / 2;
				resized.copyTo(cell(cv::Rect(ox, oy, nw, nh)));
			}

			QImage cellQImg = Utility::matToQImage(cell);
			painter.drawImage(rect.topLeft(), cellQImg);
		}

		// 编号标注
		if (showLabels)
		{
			QString label = makeLabel(i);
			int fontSize = param("label_font_size").toInt();
			QColor labelColor = Utility::arrayToColor(param("label_color").toList());
			painter.setFont(QFont(QString(), fontSize, QFont::Bold));
			painter.setPen(labelColor);
			painter.drawText(rect.x() + 4, rect.y() + fontSize + 2, label);
		}
	}

	painter.end();
	cv::Mat dst = Utility::qImageToMat(qCanvas);
	setOutput("output_image", NodeData::createImage(dst));
}
