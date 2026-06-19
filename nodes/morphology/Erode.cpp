// nodes/morphology/Erode.cpp
#include "Erode.h"
#include <opencv2/imgproc.hpp>

int Erode::morphOperation() const
{
	return cv::MORPH_ERODE;
}
