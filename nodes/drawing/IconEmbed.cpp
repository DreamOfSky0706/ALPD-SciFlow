// nodes/drawing/IconEmbed.cpp
#include "IconEmbed.h"
#include "Utility.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <QFileInfo>

void IconEmbed::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);

	addParam("source", "来源", ParamType::Combo, QString("内置图标"),
			 { {"options", QStringList{"内置图标", "自定义文件"}} });
	addParam("builtin_icon", "内置图标", ParamType::Combo, QString("数据库"),
			 { {"options", QStringList{"数据库","齿轮","灯泡","文档","人物","向右箭头","向下箭头","勾选","叉号","放大镜","图表","试管","烧杯","分子","DNA","计算机","芯片","云","锁","星形","圆形","方形","菱形","三角形","心形","信封","日历","书签","旗帜","时钟"}},
			  {"visible_when", "source==内置图标"} });
	addParam("custom_file_path", "图标文件路径", ParamType::FilePath, QString(),
			 { {"filter", "图片文件 (*.png *.svg)"}, {"visible_when", "source==自定义文件"} });
	addParam("position_x", "位置X", ParamType::Int, 100, { {"min", 0}, {"max", 20000} });
	addParam("position_y", "位置Y", ParamType::Int, 100, { {"min", 0}, {"max", 20000} });
	addParam("icon_size", "图标大小", ParamType::IntSlider, 40, { {"min", 10}, {"max", 500} });
	addParam("opacity", "不透明度", ParamType::DoubleSlider, 1.0, { {"min", 0.0}, {"max", 1.0} });
}

// 绘制内置图标到QImage
static QImage drawBuiltinIcon(const QString& name, int size)
{
	QImage img(size, size, QImage::Format_ARGB32);
	img.fill(Qt::transparent);
	QPainter p(&img); p.setRenderHint(QPainter::Antialiasing);
	int s=size, m=s/8, cx=s/2, cy=s/2;
	p.setPen(QPen(QColor(60,60,60), qMax(1,s/20)));
	p.setBrush(QColor(80,150,230));

	if(name=="数据库"){ int w=s/3; p.drawEllipse(cx-w,cy-s/4,w*2,s/2); p.drawRect(cx-w,m,w*2,cy); p.drawEllipse(cx-w,cy-s/8,w*2,s/3); }
	else if(name=="齿轮"){ for(int i=0;i<8;++i){ p.save(); p.translate(cx,cy); p.rotate(i*45); p.drawRect(-s/5,-s/14,s*2/5,s/7); p.restore(); } p.setBrush(Qt::white); p.drawEllipse(cx-s/4,cy-s/4,s/2,s/2); }
	else if(name=="灯泡"){ p.drawEllipse(cx-s/3,m,s*2/3,s/2); p.drawRect(cx-s/8,cy+s/5,s/4,s/3); }
	else if(name=="文档"){ p.drawRoundedRect(m,m,s-m*2,s-m*2,s/8,s/8); p.setBrush(Qt::white); for(int i=0;i<4;++i) p.drawLine(m+s/4, m+s/3+i*s/10, s-m-s/4, m+s/3+i*s/10); }
	else if(name=="人物"){ p.drawEllipse(cx-s/5, m, s*2/5, s/3); p.drawEllipse(cx-s/4, cy-s/8, s/2, s/2); }
	else if(name=="向右箭头"){ QPolygonF ar; ar<<QPointF(s-m,m+cy/2)<<QPointF(cx,m+cy/2)<<QPointF(cx,m)<<QPointF(m*5,cy)<<QPointF(cx,s-m)<<QPointF(cx,s-m-cy/2)<<QPointF(s-m,s-m-cy/2); p.drawPolygon(ar); }
	else if(name=="向下箭头"){ QPolygonF ar; ar<<QPointF(m+cx/2,m)<<QPointF(m+cx/2,cy)<<QPointF(m,cy)<<QPointF(cy,s-m*5)<<QPointF(s-m,cy)<<QPointF(s-m-cx/2,cy)<<QPointF(s-m-cx/2,m); p.drawPolygon(ar); }
	else if(name=="勾选"){ p.setPen(QPen(Qt::green,s/8)); p.drawLine(m,cy,s/4,s-m); p.drawLine(s/4,s-m,s-m,m); }
	else if(name=="叉号"){ p.setPen(QPen(Qt::red,s/8)); p.drawLine(m,m,s-m,s-m); p.drawLine(s-m,m,m,s-m); }
	else if(name=="放大镜"){ p.drawEllipse(m,m, s*2/3, s*2/3); p.setPen(QPen(QColor(60,60,60),s/10)); p.drawLine(s*2/3+m,s*2/3+m,s-m,s-m); }
	else if(name=="图表"){ for(int i=0;i<3;++i){ int hh=(i+1)*s/6; p.drawRect(m+i*s/4, s-m-hh, s/6, hh); } }
	else if(name=="星形"){ QPolygonF star; for(int i=0;i<10;++i){ double a=(i%2==0?0.9:0.45)*s/2; star<<QPointF(cx+a*sin(i*M_PI/5),cy-a*cos(i*M_PI/5)); } p.drawPolygon(star); }
	else if(name=="心形"){ QPainterPath hp; hp.moveTo(cx,s*7/8); hp.cubicTo(m,s/2, m,m, cx,s/4); hp.cubicTo(s-m,m, s-m,s/2, cx,s*7/8); p.drawPath(hp); }
	else if(name=="云"){ p.drawEllipse(cx-s/3,cy-s/6,s/2,s/3); p.drawEllipse(cx-s/6,cy-s/3,s/3,s/3); p.drawEllipse(cx+s/8,cy-s/4,s/3,s/2); p.drawRect(cx-s/4,cy,s/2,s/6); }
	else if(name=="锁"){ p.drawRoundedRect(m,cy,s-m*2,s-cy, s/10,s/10); p.drawEllipse(cx-s/5, m, s*2/5, s/3); p.drawRect(cx-s/8, cy-s/8, s/4, cy-m); }
	else if(name=="时钟"){ p.drawEllipse(m,m,s-m*2,s-m*2); p.drawLine(cx,cy,cx,m+s/6); p.drawLine(cx,cy,s-m-s/6,cy); }
	else if(name=="圆形"){ p.drawEllipse(m,m,s-m*2,s-m*2); }
	else if(name=="方形"){ p.drawRect(m,m,s-m*2,s-m*2); }
	else if(name=="菱形"){ QPolygonF d; d<<QPointF(cx,m)<<QPointF(s-m,cy)<<QPointF(cx,s-m)<<QPointF(m,cy); p.drawPolygon(d); }
	else if(name=="三角形"){ QPolygonF t; t<<QPointF(cx,m)<<QPointF(s-m,s-m)<<QPointF(m,s-m); p.drawPolygon(t); }
	else if(name=="信封"){ p.drawRect(m,cy-s/6,s-m*2,s/3); p.drawLine(m,cy-s/6,cx,cy+s/6); p.drawLine(cx,cy+s/6,s-m,cy-s/6); }
	else if(name=="日历"){ p.drawRoundedRect(m,m,s-m*2,s-m*2,s/10,s/10); p.setBrush(QColor(200,60,60)); p.drawRect(m,m,s-m*2,s/4); p.setBrush(QColor(60,60,60)); p.drawText(QRect(0,s/3,s,s-s/3),Qt::AlignCenter,"17"); }
	else if(name=="书签"){ p.drawRect(m,m,s/4,s-m*2); p.drawLine(s/4+m, s-m, cx, s*3/4); p.drawLine(cx, s*3/4, s-m, s-m); }
	else if(name=="旗帜"){ p.drawRect(m,m,s/10,s-m); p.drawRect(m+s/10,m,s-m-s/10,s/3); }
	else p.drawText(QRect(0,0,s,s),Qt::AlignCenter, name.left(1));
	p.end(); return img;
}

void IconEmbed::process()
{
	auto inputData = getInput("input_image");
	if (!inputData || inputData->isNull()) { reportError("输入图像为空"); return; }
	cv::Mat base = inputData->toImage();
	int targetSize = param("icon_size").toInt();
	QString source = param("source").toString();
	cv::Mat icon;

	if (source == "内置图标") {
		QString name = param("builtin_icon").toString();
		QImage qi = drawBuiltinIcon(name, targetSize);
		icon = Utility::qImageToMat(qi);
	} else {
		QString filePath = param("custom_file_path").toString();
		if (filePath.isEmpty()) { reportWarning("未指定图标文件"); setOutput("output_image", NodeData::createImage(base)); return; }
		icon = cv::imread(filePath.toStdString(), cv::IMREAD_UNCHANGED);
		if (icon.empty()) { reportError(QString("无法读取图标文件：%1").arg(filePath)); return; }
		double scale = static_cast<double>(targetSize) / icon.cols;
		cv::resize(icon, icon, cv::Size(targetSize, static_cast<int>(icon.rows * scale)));
	}

	int px = param("position_x").toInt() - icon.cols / 2;
	int py = param("position_y").toInt() - icon.rows / 2;
	double opacity = param("opacity").toDouble();
	cv::Mat dst = base.clone();
	if (dst.channels() < 3) cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);

	for (int y = 0; y < icon.rows; ++y) {
		int dy = py + y; if (dy < 0 || dy >= dst.rows) continue;
		for (int x = 0; x < icon.cols; ++x) {
			int dx = px + x; if (dx < 0 || dx >= dst.cols) continue;
			double alpha = opacity;
			if (icon.channels() == 4) alpha *= icon.at<cv::Vec4b>(y, x)[3] / 255.0;
			if (alpha <= 0) continue;
			cv::Vec3b& dp = dst.at<cv::Vec3b>(dy, dx);
			cv::Vec3b sp = icon.channels() >= 3 ? cv::Vec3b(icon.at<cv::Vec3b>(y, x))
			                                     : cv::Vec3b(icon.at<uchar>(y, x), icon.at<uchar>(y, x), icon.at<uchar>(y, x));
			for (int c = 0; c < 3; ++c) dp[c] = static_cast<uchar>(dp[c] * (1 - alpha) + sp[c] * alpha);
		}
	}
	setOutput("output_image", NodeData::createImage(dst));
}
