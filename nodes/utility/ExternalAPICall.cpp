// nodes/utility/ExternalAPICall.cpp
#include "ExternalAPICall.h"

void ExternalAPICall::defineNode()
{
	addInputPort("input_text", DataType::Text, false);
	addInputPort("input_image", DataType::Image, false);
	addOutputPort("output_image", DataType::Image);
	addOutputPort("output_text", DataType::Text);

	addParam("api_url", "API地址", ParamType::String, QString());
	addParam("method", "HTTP方法", ParamType::Combo, QString("POST"), { {"options", QStringList{"GET", "POST"}} });
	addParam("timeout", "超时(秒)", ParamType::IntSlider, 30, { {"min", 1}, {"max", 120} });
}

void ExternalAPICall::process()
{
	reportWarning("外部API调用功能尚未实现，请等待后续版本");
}
