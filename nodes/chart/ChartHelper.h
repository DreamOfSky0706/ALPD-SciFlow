// nodes/chart/ChartHelper.h
#pragma once

#include <QImage>
#include <QColor>
#include <QStringList>
#include <QVariantList>

// 图表节点的公共辅助功能
namespace ChartHelper
{

	// 默认配色方案
	inline QList<QColor> defaultPalette()
	{
		return {
			QColor("#4E79A7"), QColor("#F28E2B"), QColor("#E15759"),
			QColor("#76B7B2"), QColor("#59A14F"), QColor("#EDC948"),
			QColor("#B07AA1"), QColor("#FF9DA7"), QColor("#9C755F"),
			QColor("#BAB0AC")
		};
	}

	// 从QCustomPlot/QtCharts的QPixmap/QImage最终转为cv::Mat在各节点中单独完成

} // namespace ChartHelper
