// nodes/image_io/NoiseGenerate.h
#pragma once

#include "NodeBase.h"

// 生成随机噪声图像
class NoiseGenerate : public NodeBase
{
public:
	NoiseGenerate() = default;

	void defineNode() override;
	void process() override;

private:
	// 简化版2D Perlin噪声
	double perlinNoise(double x, double y, int seed) const;
	double fade(double t) const;
	double lerp(double a, double b, double t) const;
	double grad(int hash, double x, double y) const;
};
