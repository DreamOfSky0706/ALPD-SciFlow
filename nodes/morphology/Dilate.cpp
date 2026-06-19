// nodes/morphology/Dilate.cpp
#include "Dilate.h"
#include <opencv2/imgproc.hpp>

int Dilate::morphOperation() const
{
	return cv::MORPH_DILATE;
}
