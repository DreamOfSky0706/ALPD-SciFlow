// nodes/pattern/PatternBase.cpp
#include "PatternBase.h"
#include "Utility.h"

void PatternBase::addCommonParams()
{
	addParam("width", "宽度", ParamType::Int, 800, { {"min", 100}, {"max", 10000} });
	addParam("height", "高度", ParamType::Int, 600, { {"min", 100}, {"max", 10000} });
	addParam("primary_color", "主色", ParamType::Color, QVariantList{ 80, 150, 230, 255 });
	addParam("secondary_color", "辅色", ParamType::Color, QVariantList{ 40, 80, 120, 255 });
	addParam("background_color", "背景色", ParamType::Color, QVariantList{ 0, 0, 0, 0 });
	addParam("density", "密度", ParamType::DoubleSlider, 1.0, { {"min", 0.1}, {"max", 2.0} });
	addParam("line_width", "线宽", ParamType::IntSlider, 1, { {"min", 1}, {"max", 5} });
	addParam("seed", "随机种子", ParamType::Int, 0, { {"min", 0}, {"max", 999999} });
}

void PatternBase::getCommonValues(int& w, int& h, QColor& primary, QColor& secondary, QColor& bg, double& density, int& seed) const
{
	w = param("width").toInt();
	h = param("height").toInt();
	primary = Utility::arrayToColor(param("primary_color").toList());
	secondary = Utility::arrayToColor(param("secondary_color").toList());
	bg = Utility::arrayToColor(param("background_color").toList());
	density = param("density").toDouble();
	seed = param("seed").toInt();
}
