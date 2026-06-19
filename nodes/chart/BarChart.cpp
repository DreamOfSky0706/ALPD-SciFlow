// nodes/chart/BarChart.cpp
#include "BarChart.h"
#include "DataTable.h"
#include "Utility.h"
#include "ChartHelper.h"
#include "qcustomplot.h"

void BarChart::defineNode()
{
	addInputPort("input_data", DataType::DataTable);
	addOutputPort("output_image", DataType::Image);
	addParam("chart_width","图表宽度",ParamType::Int,800,{{"min",200},{"max",4000}});
	addParam("chart_height","图表高度",ParamType::Int,600,{{"min",200},{"max",4000}});
	addParam("title","标题",ParamType::String,QString());
	addParam("title_font_size","标题字号",ParamType::IntSlider,16,{{"min",8},{"max",48}});
	addParam("background_color","背景色",ParamType::Color,QVariantList{255,255,255,255});
	addParam("category_column","分类列",ParamType::String,QString());
	addParam("value_columns","数值列(逗号分隔)",ParamType::String,QString());
	addParam("mode","模式",ParamType::Combo,QString("分组(Grouped)"),
		{{"options",QStringList{"分组(Grouped)","堆叠(Stacked)"}}});
	addParam("x_label","X轴标签",ParamType::String,QString());
	addParam("y_label","Y轴标签",ParamType::String,QString());
	addParam("show_legend","显示图例",ParamType::Bool,true);
	addParam("bar_width_ratio","柱宽比例",ParamType::DoubleSlider,0.7,{{"min",0.1},{"max",1.0}});
}

void BarChart::process()
{
	auto d=getInput("input_data");
	if(!d||d->isNull()){reportError("输入数据为空");return;}
	auto t=d->toDataTable();
	if(!t){reportError("无法获取DataTable");return;}

	QString catCol=param("category_column").toString();
	QStringList vCols=param("value_columns").toString().split(",",Qt::SkipEmptyParts);
	for(auto&s:vCols)s=s.trimmed();
	if(!t->hasColumn(catCol)){reportError(QString("分类列[%1]不存在").arg(catCol));return;}

	int w=param("chart_width").toInt(),h=param("chart_height").toInt();
	QColor bg=Utility::arrayToColor(param("background_color").toList());
	bool stacked=param("mode").toString().startsWith("堆叠");
	double bwRatio=param("bar_width_ratio").toDouble();

	QCustomPlot plot;
	plot.setBackground(QBrush(bg));
	plot.setFixedSize(w,h);

	QString title=param("title").toString();
	if(!title.isEmpty()){
		plot.plotLayout()->insertRow(0);
		auto* te=new QCPTextElement(&plot,title);
		te->setFont(QFont(QString(),param("title_font_size").toInt(),QFont::Bold));
		plot.plotLayout()->addElement(0,0,te);
	}

	// 聚合同类目
	int catIdx=t->columnIndex(catCol);
	QMap<QString,QVector<QVector<double>>> grp;
	QStringList order;
	for(int r=0;r<t->rowCount();++r){
		QString cat=t->value(r,catIdx).toString();
		if(!grp.contains(cat)){grp[cat]=QVector<QVector<double>>(vCols.size());order<<cat;}
		for(int g=0;g<vCols.size();++g){
			bool ok;double v=t->value(r,t->columnIndex(vCols[g])).toDouble(&ok);
			grp[cat][g].append(ok?v:0.0);
		}
	}
	QVector<double> ticks; QVector<QString> labels;
	for(int i=0;i<order.size();++i){ticks<<(i+1.0);labels<<order[i];}

	auto ticker=QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);
	for(int i=0;i<labels.size();++i)ticker->addTick(ticks[i],labels[i]);
	plot.xAxis->setTicker(ticker);

	auto pal=ChartHelper::defaultPalette();
	int nG=qMax(1,vCols.size());
	double barW=stacked?bwRatio:bwRatio/nG;

	for(int g=0;g<vCols.size();++g){
		if(!t->hasColumn(vCols[g]))continue;

		QCPBars* bars=new QCPBars(plot.xAxis,plot.yAxis);
		bars->setName(vCols[g]);
		bars->setWidth(barW);
		bars->setPen(QPen(pal[g%pal.size()].darker(120)));
		bars->setBrush(pal[g%pal.size()]);

		QVector<double> vls;
		for(const auto& cat:order){
			const auto& vl=grp[cat][g];
			double sum=0;for(double v:vl)sum+=v;
			vls<<(vl.isEmpty()?0:sum/vl.size());
		}

		if(stacked&&g>0){
			for(auto* child:plot.children()){
				if(auto*b=qobject_cast<QCPBars*>(child)){
					if(b->name()==vCols[g-1]){bars->moveAbove(b);break;}
				}
			}
			bars->setData(ticks,vls);
		}else if(nG>1){
			QVector<double> sh;
			double totalW=barW*nG;
			double start=-totalW/2.0+barW/2.0;
			double off=start+g*barW;
			for(double t:ticks)sh<<t+off;
			bars->setData(sh,vls);
		}else{
			bars->setData(ticks,vls);
		}
	}

	plot.xAxis->setLabel(param("x_label").toString());
	plot.yAxis->setLabel(param("y_label").toString());
	plot.xAxis->setRange(0.3,ticks.size()+0.7);
	if(param("show_legend").toBool())plot.legend->setVisible(true);
	plot.yAxis->rescale();
	plot.replot();

	cv::Mat dst=Utility::qImageToMat(plot.toPixmap(w,h).toImage());
	setOutput("output_image",NodeData::createImage(dst));
}
