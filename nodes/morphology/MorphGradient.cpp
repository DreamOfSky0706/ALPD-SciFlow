// nodes/morphology/MorphGradient.cpp
#include "MorphGradient.h"
#include <opencv2/imgproc.hpp>

int MorphGradient::morphOperation() const
{
	return cv::MORPH_GRADIENT;
}
