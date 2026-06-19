// nodes/morphology/MorphOpen.cpp
#include "MorphOpen.h"
#include <opencv2/imgproc.hpp>

int MorphOpen::morphOperation() const
{
	return cv::MORPH_OPEN;
}
