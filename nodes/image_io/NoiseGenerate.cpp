// nodes/image_io/NoiseGenerate.cpp
#include "NoiseGenerate.h"
#include <opencv2/core.hpp>
#include <cmath>
#include <random>
#include <numeric>

void NoiseGenerate::defineNode()
{
	addOutputPort("output_image", DataType::Image);

	addParam("width", "宽度", ParamType::Int, 512, { {"min", 1}, {"max", 20000} });
	addParam("height", "高度", ParamType::Int, 512, { {"min", 1}, {"max", 20000} });
	addParam("noise_type", "噪声类型", ParamType::Combo, QString("Perlin噪声"),
			 { {"options", QStringList{"高斯噪声", "均匀噪声", "Perlin噪声"}} });
	addParam("seed", "随机种子", ParamType::Int, 0,
			 { {"min", 0}, {"max", 999999} });
	addParam("scale", "频率", ParamType::DoubleSlider, 0.05,
			 { {"min", 0.001}, {"max", 1.0}, {"visible_when", "noise_type==Perlin噪声"} });
	addParam("octaves", "叠加层数", ParamType::IntSlider, 4,
			 { {"min", 1}, {"max", 8}, {"visible_when", "noise_type==Perlin噪声"} });
}

void NoiseGenerate::process()
{
	int w = param("width").toInt();
	int h = param("height").toInt();
	QString noiseType = param("noise_type").toString();
	int seed = param("seed").toInt();

	if (w < 1 || w > 20000 || h < 1 || h > 20000)
	{
		reportError(QString("图像尺寸超出范围：%1x%2").arg(w).arg(h));
		return;
	}

	cv::Mat img(h, w, CV_8UC1);

	if (seed == 0)
	{
		seed = static_cast<int>(std::random_device{}());
	}

	if (noiseType == "高斯噪声")
	{
		cv::Mat noise(h, w, CV_64FC1);
		cv::RNG rng(seed);
		rng.fill(noise, cv::RNG::NORMAL, 128.0, 40.0);
		noise.convertTo(img, CV_8UC1);
	}
	else if (noiseType == "均匀噪声")
	{
		cv::RNG rng(seed);
		rng.fill(img, cv::RNG::UNIFORM, 0, 256);
	}
	else
	{
		// Perlin噪声
		double scale = param("scale").toDouble();
		int octaves = param("octaves").toInt();

		for (int y = 0; y < h; ++y)
		{
			uchar* row = img.ptr<uchar>(y);
			for (int x = 0; x < w; ++x)
			{
				double val = 0.0;
				double amplitude = 1.0;
				double frequency = scale;
				double maxVal = 0.0;

				for (int o = 0; o < octaves; ++o)
				{
					val += amplitude * perlinNoise(x * frequency, y * frequency, seed + o * 1000);
					maxVal += amplitude;
					amplitude *= 0.5;
					frequency *= 2.0;
				}

				val /= maxVal;
				// 映射到0-255
				int pixel = static_cast<int>((val + 1.0) * 0.5 * 255.0);
				pixel = std::max(0, std::min(255, pixel));
				row[x] = static_cast<uchar>(pixel);
			}
		}
	}

	setOutput("output_image", NodeData::createImage(img));
}

double NoiseGenerate::perlinNoise(double x, double y, int seed) const
{
	// 使用seed生成置换表
	static thread_local std::vector<int> perm;
	static thread_local int lastSeed = -1;

	if (lastSeed != seed || perm.empty())
	{
		perm.resize(512);
		std::vector<int> base(256);
		std::iota(base.begin(), base.end(), 0);
		std::mt19937 gen(seed);
		std::shuffle(base.begin(), base.end(), gen);
		for (int i = 0; i < 256; ++i)
		{
			perm[i] = base[i];
			perm[i + 256] = base[i];
		}
		lastSeed = seed;
	}

	int xi = static_cast<int>(std::floor(x)) & 255;
	int yi = static_cast<int>(std::floor(y)) & 255;
	double xf = x - std::floor(x);
	double yf = y - std::floor(y);

	double u = fade(xf);
	double v = fade(yf);

	int aa = perm[perm[xi] + yi];
	int ab = perm[perm[xi] + yi + 1];
	int ba = perm[perm[xi + 1] + yi];
	int bb = perm[perm[xi + 1] + yi + 1];

	double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1.0, yf), u);
	double x2 = lerp(grad(ab, xf, yf - 1.0), grad(bb, xf - 1.0, yf - 1.0), u);

	return lerp(x1, x2, v);
}

double NoiseGenerate::fade(double t) const
{
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double NoiseGenerate::lerp(double a, double b, double t) const
{
	return a + t * (b - a);
}

double NoiseGenerate::grad(int hash, double x, double y) const
{
	int h = hash & 3;
	double u = (h < 2) ? x : y;
	double v = (h < 2) ? y : x;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}
