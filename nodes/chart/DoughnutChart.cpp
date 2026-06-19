// nodes/chart/DoughnutChart.cpp
#include "DoughnutChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include <QPainter>
#include <QImage>
#include <cmath>

void DoughnutChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);
	addParam("chart_width", "图表宽度", ParamType::Int, 800, {{"min",200},{"max",4000}});
	addParam("chart_height", "图表高度", ParamType::Int, 600, {{"min",200},{"max",4000}});
	addParam("title", "标题", ParamType::String, QString());
	addParam("title_font_size", "标题字号", ParamType::IntSlider, 16, {{"min",8},{"max",48}});
	addParam("background_color", "背景色", ParamType::Color, QVariantList{255,255,255,255});
	addParam("label_column", "标签列", ParamType::String, QString());
	addParam("value_column", "数值列", ParamType::String, QString());
	addParam("show_legend", "显示图例", ParamType::Bool, true);
	addParam("hole_ratio", "镂空比例", ParamType::DoubleSlider, 0.5, {{"min",0.1},{"max",0.8}});
}

void DoughnutChart::process()
{
	auto data = getInput("input_data");
	if (!data || data->isNull()) { reportError("输入数据为空"); return; }
	auto table = data->toDataTable();
	if (!table) { reportError("无法获取DataTable"); return; }

	QString labelCol = param("label_column").toString();
	QString valueCol = param("value_column").toString();
	if (!table->hasColumn(labelCol) || !table->hasColumn(valueCol)) {
		reportError("指定的列不存在"); return;
	}

	int w = param("chart_width").toInt(), h = param("chart_height").toInt();
	QColor bg = Utility::arrayToColor(param("background_color").toList());
	double hole = param("hole_ratio").toDouble();
	auto palette = ChartHelper::defaultPalette();

	// 聚合
	QMap<QString,double> agg; QStringList order;
	for (int r=0; r<table->rowCount(); ++r) {
		QString l=table->value(r,table->columnIndex(labelCol)).toString();
		bool ok; double v=table->value(r,table->columnIndex(valueCol)).toDouble(&ok);
		if(!ok) v=0; if(!agg.contains(l)) order<<l; agg[l]+=v;
	}
	double total=0; for(auto v:agg) total+=v;
	if(total<=0){ reportError("所有数值为零"); return; }

	QImage img(w,h,QImage::Format_ARGB32); img.fill(bg);
	QPainter p(&img); p.setRenderHint(QPainter::Antialiasing);

	QString title=param("title").toString();
	int tfs=param("title_font_size").toInt();
	if(!title.isEmpty()){ p.setFont(QFont(QString(),tfs,QFont::Bold)); p.setPen(Qt::black);
		p.drawText(QRect(0,5,w,tfs+10),Qt::AlignHCenter|Qt::AlignTop,title); }
	int th=title.isEmpty()?0:tfs+15;
	int sz=qMin(w-40,h-40-th), cx=w/2, cy=th+(h-th)/2, r=sz/2-10, ir=(int)(r*hole);

	double sa=90.0*16;
	for(int i=0;i<order.size();++i){
		double val=agg[order[i]], span=val/total*360.0*16;
		QColor c=palette[i%palette.size()]; p.setBrush(c); p.setPen(QPen(c.darker(120),1));
		QRectF outer(cx-r,cy-r,r*2,r*2), inner(cx-ir,cy-ir,ir*2,ir*2);
		// 用 arc 路径画环形扇区
		QPainterPath path; path.arcTo(outer,sa/16.0,span/16.0); path.arcTo(inner,(sa+span)/16.0,-span/16.0); path.closeSubpath();
		p.drawPath(path);
		if(param("show_legend").toBool()){
			int ly=th+20+i*22; p.setBrush(c); p.setPen(Qt::NoPen); p.drawRect(w-120,ly,14,14);
			p.setPen(Qt::black); p.setFont(QFont(QString(),10));
			p.drawText(w-104,ly,100,14,Qt::AlignLeft|Qt::AlignVCenter,QString("%1 (%2%)").arg(order[i]).arg(100*val/total,0,'f',1));
		}
		sa+=span;
	}
	p.end();
	setOutput("output_image", NodeData::createImage(Utility::qImageToMat(img)));
}
