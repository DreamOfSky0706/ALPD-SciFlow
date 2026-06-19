// nodes/filter/Clahe.cpp
#include "Clahe.h"
#include <opencv2/imgproc.hpp>

void Clahe::defineNode()
{
	addInputPort("input_image", DataType::Image);
	addOutputPort("output_image", DataType::Image);
	addParam("clip_limit","对比度限制",ParamType::DoubleSlider,2.0,{{"min",0.1},{"max",10.0}});
	addParam("tile_size","网格尺寸",ParamType::IntSlider,8,{{"min",1},{"max",32}});
}

void Clahe::process()
{
	auto data=getInput("input_image");
	if(!data||data->isNull()){reportError("输入图像为空");return;}
	cv::Mat src=data->toImage();
	// 确保8位深度
	if(src.depth()!=CV_8U) src.convertTo(src,CV_8U,src.depth()==CV_16U?1.0/256.0:1.0);
	double clip=param("clip_limit").toDouble();
	int tile=param("tile_size").toInt();
	cv::Mat dst;

	if(src.channels()==1){
		auto clahe=cv::createCLAHE(clip,cv::Size(tile,tile));
		clahe->apply(src,dst);
	}else{
		cv::Mat bgr,lab;
		if(src.channels()==4) cv::cvtColor(src,bgr,cv::COLOR_BGRA2BGR);
		else bgr=src;
		cv::cvtColor(bgr,lab,cv::COLOR_BGR2Lab);
		std::vector<cv::Mat> chs; cv::split(lab,chs);
		chs[0].convertTo(chs[0],CV_8U); // 确保L通道为8位
		auto clahe=cv::createCLAHE(clip,cv::Size(tile,tile));
		clahe->apply(chs[0],chs[0]);
		cv::merge(chs,lab);
		cv::cvtColor(lab,dst,cv::COLOR_Lab2BGR);
	}
	setOutput("output_image",NodeData::createImage(dst));
}
