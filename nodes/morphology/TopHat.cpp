// nodes/morphology/TopHat.cpp
#include "TopHat.h"
#include <opencv2/imgproc.hpp>

int TopHat::morphOperation() const
{
	return cv::MORPH_TOPHAT;
}
