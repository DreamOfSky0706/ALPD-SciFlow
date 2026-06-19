// nodes/text/ArtisticText.cpp
#include "ArtisticText.h"
#include "Utility.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetricsF>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QImage>
#include <opencv2/imgproc.hpp>

void ArtisticText::defineNode()
{
	addInputPort("input_image", DataType::Image, false);
	addOutputPort("output_image", DataType::Image);

	addParam("text", "文本内容", ParamType::String, QString("标题"));
	addParam("font_family", "字体", ParamType::Font, QString());
	addParam("font_size", "字号", ParamType::IntSlider, 72, { {"min", 12}, {"max", 800} });
	addParam("fill_mode", "填充模式", ParamType::Combo, QString("纯色"),
			 { {"options", QStringList{"纯色", "水平渐变", "垂直渐变", "径向渐变"}} });
	addParam("fill_color1", "填充色1", ParamType::Color, QVariantList{ 255, 255, 255, 255 });
	addParam("fill_color2", "填充色2", ParamType::Color,
			 QVariantList{ 0, 0, 0, 255 },
			 { {"visible_when", "fill_mode!=纯色"} });
	addParam("stroke_color", "描边颜色", ParamType::Color, QVariantList{ 0, 0, 0, 255 });
	addParam("stroke_width", "描边宽度", ParamType::IntSlider, 2, { {"min", 0}, {"max", 20} });
	addParam("shadow_enabled", "启用阴影", ParamType::Bool, true);
	addParam("shadow_color", "阴影颜色", ParamType::Color, QVariantList{ 0, 0, 0, 128 });
	addParam("shadow_offset_x", "阴影偏移X", ParamType::IntSlider, 3,
			 { {"min", -20}, {"max", 20} });
	addParam("shadow_offset_y", "阴影偏移Y", ParamType::IntSlider, 3,
			 { {"min", -20}, {"max", 20} });
	addParam("shadow_blur", "阴影模糊", ParamType::IntSlider, 5,
			 { {"min", 0}, {"max", 20} });
	addParam("letter_spacing", "字间距", ParamType::IntSlider, 0,
			 { {"min", -10}, {"max", 50} });
	addParam("position_x", "位置X(-1居中)", ParamType::Int, -1,
			 { {"min", -1}, {"max", 20000} });
	addParam("position_y", "位置Y(-1居中)", ParamType::Int, -1,
			 { {"min", -1}, {"max", 20000} });
}

void ArtisticText::process()
{
	QString text = param("text").toString();
	if (text.isEmpty())
	{
		reportError("文本内容为空");
		return;
	}

	QString fontFamily = param("font_family").toString();
	int fontSize = param("font_size").toInt();
	int letterSpacing = param("letter_spacing").toInt();

	QFont font(fontFamily, fontSize);
	font.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);

	QFontMetricsF fm(font);
	QRectF textBounds = fm.boundingRect(text);
	// 使用实际渲染区域而非boundingRect确保不截断
	qreal textH = fm.height() + fm.lineSpacing() * (text.count('\n'));
	int shadowBlur = param("shadow_blur").toInt();
	int strokeWidth = param("stroke_width").toInt();
	int padding = shadowBlur + strokeWidth + 20;

	int canvasW = static_cast<int>(textBounds.width()) + padding * 2;
	int canvasH = static_cast<int>(qMax(textBounds.height(), textH)) + padding * 2 + 10;

	// 渲染文字到透明图像
	QImage textCanvas(canvasW, canvasH, QImage::Format_RGBA8888);
	textCanvas.fill(Qt::transparent);

	QPainterPath textPath;
	// 基线 = padding + fm.ascent()（标准基线位置）
	textPath.addText(padding, padding + fm.ascent(), font, text);

	// 阴影层
	bool shadowEnabled = param("shadow_enabled").toBool();
	if (shadowEnabled)
	{
		int sox = param("shadow_offset_x").toInt();
		int soy = param("shadow_offset_y").toInt();
		QColor shadowColor = Utility::arrayToColor(param("shadow_color").toList());

		QImage shadowImg(canvasW, canvasH, QImage::Format_RGBA8888);
		shadowImg.fill(Qt::transparent);
		QPainter sp(&shadowImg);
		sp.setRenderHint(QPainter::Antialiasing);
		QPainterPath shadowPath = textPath;
		shadowPath.translate(sox, soy);
		sp.setBrush(shadowColor);
		sp.setPen(Qt::NoPen);
		sp.drawPath(shadowPath);
		sp.end();

		// 简化的模糊：多次缩小再放大
		if (shadowBlur > 0)
		{
			cv::Mat shadowMat = Utility::qImageToMat(shadowImg);
			int ksize = shadowBlur * 2 + 1;
			cv::GaussianBlur(shadowMat, shadowMat, cv::Size(ksize, ksize), 0);
			shadowImg = Utility::matToQImage(shadowMat);
		}

		QPainter cp(&textCanvas);
		cp.drawImage(0, 0, shadowImg);
		cp.end();
	}

	// 文字描边和填充
	QPainter painter(&textCanvas);
	painter.setRenderHint(QPainter::Antialiasing);

	// 描边
	if (strokeWidth > 0)
	{
		QColor strokeColor = Utility::arrayToColor(param("stroke_color").toList());
		painter.setPen(QPen(strokeColor, strokeWidth * 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter.drawPath(textPath);
	}

	// 填充
	QString fillMode = param("fill_mode").toString();
	QColor c1 = Utility::arrayToColor(param("fill_color1").toList());
	QColor c2 = Utility::arrayToColor(param("fill_color2").toList());

	if (fillMode == "纯色")
	{
		painter.setBrush(c1);
	}
	else if (fillMode == "水平渐变")
	{
		QLinearGradient grad(textPath.boundingRect().left(), 0,
							 textPath.boundingRect().right(), 0);
		grad.setColorAt(0, c1);
		grad.setColorAt(1, c2);
		painter.setBrush(grad);
	}
	else if (fillMode == "垂直渐变")
	{
		QLinearGradient grad(0, textPath.boundingRect().top(),
							 0, textPath.boundingRect().bottom());
		grad.setColorAt(0, c1);
		grad.setColorAt(1, c2);
		painter.setBrush(grad);
	}
	else
	{
		QRectF br = textPath.boundingRect();
		QRadialGradient grad(br.center(), std::max(br.width(), br.height()) / 2.0);
		grad.setColorAt(0, c1);
		grad.setColorAt(1, c2);
		painter.setBrush(grad);
	}

	painter.setPen(Qt::NoPen);
	painter.drawPath(textPath);
	painter.end();

	// 判断是否有底图输入
	auto inputData = getInput("input_image");
	if (inputData && !inputData->isNull())
	{
		cv::Mat bgImg = inputData->toImage();
		QImage bgQImg = Utility::matToQImage(bgImg);
		if (bgQImg.format() != QImage::Format_RGB888)
		{
			bgQImg = bgQImg.convertToFormat(QImage::Format_RGB888);
		}

		int posX = param("position_x").toInt();
		int posY = param("position_y").toInt();
		if (posX < 0) posX = (bgQImg.width() - textCanvas.width()) / 2;
		if (posY < 0) posY = (bgQImg.height() - textCanvas.height()) / 2;

		QPainter bgPainter(&bgQImg);
		bgPainter.drawImage(posX, posY, textCanvas);
		bgPainter.end();

		cv::Mat result = Utility::qImageToMat(bgQImg);
		setOutput("output_image", NodeData::createImage(result));
	}
	else
	{
		cv::Mat result = Utility::qImageToMat(textCanvas);
		setOutput("output_image", NodeData::createImage(result));
	}
}
