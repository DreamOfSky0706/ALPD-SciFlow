// nodes/morphology/BlackHat.cpp
#include "BlackHat.h"
#include <opencv2/imgproc.hpp>

int BlackHat::morphOperation() const
{
	return cv::MORPH_BLACKHAT;
}
