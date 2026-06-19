// nodes/morphology/MorphClose.cpp
#include "MorphClose.h"
#include <opencv2/imgproc.hpp>

int MorphClose::morphOperation() const
{
	return cv::MORPH_CLOSE;
}
