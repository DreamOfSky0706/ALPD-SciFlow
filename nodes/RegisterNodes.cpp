// nodes/RegisterNodes.cpp
#include "RegisterNodes.h"
#include "NodeFactory.h"

// 输入输出 - 新增文本和数值节点
#include "filter/Clahe.h"
#include "data_proc/DataPivot.h"
#include "data_proc/DataProfile.h"
#include "layout/ImageDiff.h"
#include "stylize/EmbossEffect.h"
#include "image_io/ForEachImage.h"
#include "image_io/ImageCollector.h"
#include "text_io/TextFileRead.h"
#include "text_io/TextFileWrite.h"
#include "text_io/NumericWrite.h"

// 输入输出/图像
#include "image_io/ImageFileRead.h"
#include "image_io/BatchImageRead.h"
#include "image_io/SolidColorGenerate.h"
#include "image_io/GradientGenerate.h"
#include "image_io/CheckerboardGenerate.h"
#include "image_io/NoiseGenerate.h"
#include "image_io/ImageFileWrite.h"
#include "image_io/BatchImageWrite.h"

// 几何变换
#include "geometry/Resize.h"
#include "geometry/Crop.h"
#include "geometry/Rotate.h"
#include "geometry/Flip.h"
#include "geometry/AffineTransform.h"
#include "geometry/PerspectiveTransform.h"

// 色彩调整
#include "color/BrightnessContrast.h"
#include "color/Invert.h"
#include "color/Levels.h"
#include "color/HueShift.h"
#include "color/SaturationAdjust.h"
#include "color/LightnessAdjust.h"
#include "color/ColorBalance.h"
#include "color/Curves.h"
#include "color/WhiteBalance.h"

// 滤波与模糊
#include "filter/GaussianBlur.h"
#include "filter/MedianFilter.h"
#include "filter/BilateralFilter.h"
#include "filter/MotionBlur.h"
#include "filter/Sharpen.h"
#include "filter/CustomKernel.h"

// 形态学操作
#include "morphology/Dilate.h"
#include "morphology/Erode.h"
#include "morphology/MorphOpen.h"
#include "morphology/MorphClose.h"
#include "morphology/MorphGradient.h"
#include "morphology/TopHat.h"
#include "morphology/BlackHat.h"

// 边缘检测与分割
#include "edge/CannyEdge.h"
#include "edge/Sobel.h"
#include "edge/Laplacian.h"
#include "edge/GlobalThreshold.h"
#include "edge/AdaptiveThreshold.h"
#include "edge/OtsuThreshold.h"
#include "edge/FindContours.h"

// 通道操作
#include "channel/ChannelSplit.h"
#include "channel/ChannelMerge.h"
#include "channel/ConvertToGray.h"
#include "channel/ConvertToHSV.h"
#include "channel/ConvertToLab.h"
#include "channel/ConvertToYCrCb.h"
#include "channel/ExtractAlpha.h"
#include "channel/SetAlpha.h"

// 图层与合成
#include "composite/Blend.h"
#include "composite/MaskBlend.h"
#include "composite/OpacityAdjust.h"
#include "composite/ImageStack.h"
#include "composite/ImageGridStitch.h"
#include "composite/ImageWatermark.h"
#include "composite/TextWatermark.h"

// 文字与标注
#include "text/TextOverlay.h"
#include "text/ArtisticText.h"
#include "text/Annotation.h"
#include "text/ScaleBar.h"
#include "text/NumberLabel.h"
#include "text/LegendOverlay.h"

// 输入输出/数据
#include "data_io/CSVRead.h"
#include "data_io/ExcelRead.h"
#include "data_io/JSONDataRead.h"
#include "data_io/ManualDataInput.h"
#include "data_io/DataExport.h"

// 数据处理
#include "data_proc/DataFilter.h"
#include "data_proc/HandleMissing.h"
#include "data_proc/RemoveDuplicates.h"
#include "data_proc/DataSort.h"
#include "data_proc/DataNormalize.h"
#include "data_proc/ColumnCalculate.h"
#include "data_proc/DataAggregate.h"

// 图表生成
#include "chart/LineChart.h"
#include "chart/AreaChart.h"
#include "chart/BarChart.h"
#include "chart/HorizontalBarChart.h"
#include "chart/PieChart.h"
#include "chart/DoughnutChart.h"
#include "chart/ScatterPlot.h"
#include "chart/Histogram.h"
#include "chart/BoxPlot.h"
#include "chart/Heatmap.h"
#include "chart/RadarChart.h"

// 风格化与特效
#include "stylize/OilPaintEffect.h"
#include "stylize/SketchEffect.h"
#include "stylize/Pixelate.h"
#include "stylize/GradientMap.h"
#include "stylize/VintageEffect.h"

// 版面排版与装饰
#include "layout/MultiImageLayout.h"
#include "layout/CanvasExtend.h"
#include "layout/BorderDecoration.h"
#include "layout/DropShadow.h"
#include "layout/ImagePreview.h"

// 图形绘制与流程图
#include "drawing/BlankCanvas.h"
#include "drawing/DrawRectangle.h"
#include "drawing/DrawRoundedRect.h"
#include "drawing/DrawEllipse.h"
#include "drawing/DrawDiamond.h"
#include "drawing/DrawParallelogram.h"
#include "drawing/DrawArrow.h"
#include "drawing/DrawPolyline.h"
#include "drawing/DrawCurve.h"
#include "drawing/FlowchartElement.h"
#include "drawing/FlowchartAutoLayout.h"
#include "drawing/IconEmbed.h"

// 实用工具与高级
#include "utility/ImageInfo.h"
#include "utility/ImageHistogramStats.h"
#include "utility/DataQualityScore.h"
#include "utility/TableInfo.h"
#include "utility/TextInfo.h"
#include "utility/DataConditionalRouter.h"
#include "utility/ConditionalRouter.h"
#include "utility/ListIterator.h"
#include "utility/ExternalScript.h"
#include "utility/ExternalAPICall.h"

// 几何图案生成
#include "pattern/GenericPattern.h"
#include "pattern/CSPattern.h"
#include "pattern/ChemistryPattern.h"
#include "pattern/BioPattern.h"
#include "pattern/SocialSciPattern.h"
#include "pattern/CustomAssetImport.h"

void registerAllNodes()
{
	auto& f = NodeFactory::instance();

	// 输入输出/图像 - 文件读取
	f.registerNode<ImageFileRead>("ImageFileRead", "输入输出/图像/读取", "图片文件读取");
	f.registerNode<BatchImageRead>("BatchImageRead", "输入输出/图像/读取", "批量图片读取");

	// 输入输出/图像 - 图像生成
	f.registerNode<SolidColorGenerate>("SolidColorGenerate", "图像生成", "纯色图生成");
	f.registerNode<GradientGenerate>("GradientGenerate", "图像生成", "渐变图生成");
	f.registerNode<CheckerboardGenerate>("CheckerboardGenerate", "图像生成", "棋盘格生成");
	f.registerNode<NoiseGenerate>("NoiseGenerate", "图像生成", "噪声图生成");

	// 输入输出/图像 - 文件导出
	f.registerNode<ImageFileWrite>("ImageFileWrite", "输入输出/图像/导出", "图片文件导出");
	f.registerNode<BatchImageWrite>("BatchImageWrite", "输入输出/图像/导出", "批量图片导出");
	f.registerNode<ImageCollector>("ImageCollector", "输入输出/图像/导出", "图片收集成批");
		f.registerNode<ForEachImage>("ForEachImage", "输入输出/图像/导出", "批次循环处理");

	// 几何变换 - 基础变换
	f.registerNode<Resize>("Resize", "几何变换", "缩放");
	f.registerNode<Crop>("Crop", "几何变换", "裁剪");
	f.registerNode<Rotate>("Rotate", "几何变换", "旋转");
	f.registerNode<Flip>("Flip", "几何变换", "翻转");

	// 几何变换 - 高级变换
	f.registerNode<AffineTransform>("AffineTransform", "几何变换", "仿射变换");
	f.registerNode<PerspectiveTransform>("PerspectiveTransform", "几何变换", "透视变换");

	// 色彩调整 - 基础调整
	f.registerNode<BrightnessContrast>("BrightnessContrast", "色彩调整/基础", "亮度/对比度");
	f.registerNode<Invert>("Invert", "色彩调整/基础", "反色");
	f.registerNode<Levels>("Levels", "色彩调整/基础", "色阶");

	// 色彩调整 - HSL调整
	f.registerNode<HueShift>("HueShift", "色彩调整/HSL", "色相偏移");
	f.registerNode<SaturationAdjust>("SaturationAdjust", "色彩调整/HSL", "饱和度调整");
	f.registerNode<LightnessAdjust>("LightnessAdjust", "色彩调整/HSL", "明度调整");

	// 色彩调整 - 高级调整
	f.registerNode<ColorBalance>("ColorBalance", "色彩调整/高级", "色彩平衡");
	f.registerNode<Curves>("Curves", "色彩调整/高级", "曲线调整");
	f.registerNode<WhiteBalance>("WhiteBalance", "色彩调整/高级", "色温/白平衡");

	// 滤波与模糊 - 模糊
	f.registerNode<GaussianBlur>("GaussianBlur", "滤波与模糊/模糊", "高斯模糊");
	f.registerNode<MedianFilter>("MedianFilter", "滤波与模糊/模糊", "中值滤波");
	f.registerNode<BilateralFilter>("BilateralFilter", "滤波与模糊/模糊", "双边滤波");
	f.registerNode<MotionBlur>("MotionBlur", "滤波与模糊/模糊", "运动模糊");

	// 滤波与模糊 - 锐化
	f.registerNode<Sharpen>("Sharpen", "滤波与模糊/锐化", "USM锐化");
	f.registerNode<CustomKernel>("CustomKernel", "滤波与模糊/锐化", "自定义卷积核");
		f.registerNode<Clahe>("Clahe", "滤波与模糊/增强", "CLAHE自适应均衡化");

	// 形态学操作 - 基本运算
	f.registerNode<Dilate>("Dilate", "形态学操作/基本", "膨胀");
	f.registerNode<Erode>("Erode", "形态学操作/基本", "腐蚀");

	// 形态学操作 - 复合运算
	f.registerNode<MorphOpen>("MorphOpen", "形态学操作/复合", "开运算");
	f.registerNode<MorphClose>("MorphClose", "形态学操作/复合", "闭运算");
	f.registerNode<MorphGradient>("MorphGradient", "形态学操作/复合", "形态学梯度");
	f.registerNode<TopHat>("TopHat", "形态学操作/复合", "顶帽变换");
	f.registerNode<BlackHat>("BlackHat", "形态学操作/复合", "黑帽变换");

	// 边缘检测与分割 - 边缘检测
	f.registerNode<CannyEdge>("CannyEdge", "边缘检测与分割/检测", "Canny边缘检测");
	f.registerNode<Sobel>("Sobel", "边缘检测与分割/检测", "Sobel算子");
	f.registerNode<Laplacian>("Laplacian", "边缘检测与分割/检测", "Laplacian算子");

	// 边缘检测与分割 - 阈值分割
	f.registerNode<GlobalThreshold>("GlobalThreshold", "边缘检测与分割/阈值", "全局阈值");
	f.registerNode<AdaptiveThreshold>("AdaptiveThreshold", "边缘检测与分割/阈值", "自适应阈值");
	f.registerNode<OtsuThreshold>("OtsuThreshold", "边缘检测与分割/阈值", "Otsu自动阈值");

	// 边缘检测与分割 - 轮廓
	f.registerNode<FindContours>("FindContours", "边缘检测与分割/轮廓", "轮廓检测与绘制");

	// 通道操作 - 拆分合并
	f.registerNode<ChannelSplit>("ChannelSplit", "通道操作/拆分合并", "通道拆分");
	f.registerNode<ChannelMerge>("ChannelMerge", "通道操作/拆分合并", "通道合并");

	// 通道操作 - 空间转换
	f.registerNode<ConvertToGray>("ConvertToGray", "通道操作/转换", "转灰度");
	f.registerNode<ConvertToHSV>("ConvertToHSV", "通道操作/转换", "转HSV");
	f.registerNode<ConvertToLab>("ConvertToLab", "通道操作/转换", "转Lab");
	f.registerNode<ConvertToYCrCb>("ConvertToYCrCb", "通道操作/转换", "转YCrCb");

	// 通道操作 - Alpha
	f.registerNode<ExtractAlpha>("ExtractAlpha", "通道操作/Alpha", "提取Alpha通道");
	f.registerNode<SetAlpha>("SetAlpha", "通道操作/Alpha", "设置Alpha通道");

	// 图层与合成 - 混合
	f.registerNode<Blend>("Blend", "图层与合成/混合", "图像混合");
	f.registerNode<MaskBlend>("MaskBlend", "图层与合成/混合", "蒙版混合");
	f.registerNode<OpacityAdjust>("OpacityAdjust", "图层与合成/混合", "透明度调整");

	// 图层与合成 - 拼接
	f.registerNode<ImageStack>("ImageStack", "图层与合成/拼接", "图像堆叠");
	f.registerNode<ImageGridStitch>("ImageGridStitch", "图层与合成/拼接", "图像网格拼接");

	// 图层与合成 - 水印
	f.registerNode<ImageWatermark>("ImageWatermark", "图层与合成/水印", "图片水印叠加");
	f.registerNode<TextWatermark>("TextWatermark", "图层与合成/水印", "文字水印叠加");

	// 文字与标注 - 文字
	f.registerNode<TextOverlay>("TextOverlay", "文字与标注/文字", "文字叠加");
	f.registerNode<ArtisticText>("ArtisticText", "文字与标注/文字", "艺术字生成");

	// 文字与标注 - 科研标注
	f.registerNode<Annotation>("Annotation", "文字与标注/科研标注", "箭头与标注");
	f.registerNode<ScaleBar>("ScaleBar", "文字与标注/科研标注", "比例尺绘制");
	f.registerNode<NumberLabel>("NumberLabel", "文字与标注/科研标注", "编号标注");
	f.registerNode<LegendOverlay>("LegendOverlay", "文字与标注/科研标注", "图例叠加");

	// 输入输出/数据 - 读取
	f.registerNode<CSVRead>("CSVRead", "输入输出/数据/读取", "CSV文件读取");
	f.registerNode<ExcelRead>("ExcelRead", "输入输出/数据/读取", "Excel文件读取");
	f.registerNode<JSONDataRead>("JSONDataRead", "输入输出/数据/读取", "JSON数据读取");
	f.registerNode<ManualDataInput>("ManualDataInput", "输入输出/数据/读取", "手动数据输入");

	// 输入输出/数据 - 导出
	f.registerNode<DataExport>("DataExport", "输入输出/数据/导出", "数据导出");

		// 输入输出 - 文本
		f.registerNode<TextFileRead>("TextFileRead", "输入输出/文本/读取", "文本文件读取");
		f.registerNode<TextFileWrite>("TextFileWrite", "输入输出/文本/导出", "文本文件导出");

		// 输入输出 - 数值
		f.registerNode<NumericWrite>("NumericWrite", "输入输出/文本/导出", "数值导出");

	// 数据处理 - 清洗
	f.registerNode<DataFilter>("DataFilter", "数据处理/清洗", "数据过滤");
	f.registerNode<HandleMissing>("HandleMissing", "数据处理/清洗", "缺失值处理");
	f.registerNode<RemoveDuplicates>("RemoveDuplicates", "数据处理/清洗", "去重");

	// 数据处理 - 变换
	f.registerNode<DataSort>("DataSort", "数据处理/变换", "数据排序");
	f.registerNode<DataNormalize>("DataNormalize", "数据处理/变换", "数据归一化");
	f.registerNode<ColumnCalculate>("ColumnCalculate", "数据处理/变换", "列运算");

	// 数据处理 - 统计
	f.registerNode<DataAggregate>("DataAggregate", "数据处理/统计", "数据聚合统计");

		// 数据处理 - 高级
		f.registerNode<DataPivot>("DataPivot", "数据处理/高级", "数据透视表");
		f.registerNode<DataProfile>("DataProfile", "数据处理/高级", "数据画像");

	// 图表生成 - 趋势类
	f.registerNode<LineChart>("LineChart", "图表生成/趋势类", "折线图");
	f.registerNode<AreaChart>("AreaChart", "图表生成/趋势类", "面积图");

	// 图表生成 - 对比类
	f.registerNode<BarChart>("BarChart", "图表生成/对比类", "柱状图");
	f.registerNode<HorizontalBarChart>("HorizontalBarChart", "图表生成/对比类", "条形图");

	// 图表生成 - 占比类
	f.registerNode<PieChart>("PieChart", "图表生成/占比类", "饼图");
	f.registerNode<DoughnutChart>("DoughnutChart", "图表生成/占比类", "环形图");

	// 图表生成 - 分布类
	f.registerNode<ScatterPlot>("ScatterPlot", "图表生成/分布类", "散点图");
	f.registerNode<Histogram>("Histogram", "图表生成/分布类", "直方图");
	f.registerNode<BoxPlot>("BoxPlot", "图表生成/分布类", "箱线图");

	// 图表生成 - 高级
	f.registerNode<Heatmap>("Heatmap", "图表生成/高级", "热力图");
	f.registerNode<RadarChart>("RadarChart", "图表生成/高级", "雷达图");

	// 风格化与特效 - 艺术效果
	f.registerNode<OilPaintEffect>("OilPaintEffect", "风格化与特效/艺术效果", "油画效果");
	f.registerNode<SketchEffect>("SketchEffect", "风格化与特效/艺术效果", "素描效果");
	f.registerNode<Pixelate>("Pixelate", "风格化与特效/艺术效果", "像素化/马赛克");

	// 风格化与特效 - 色调效果
	f.registerNode<GradientMap>("GradientMap", "风格化与特效/色调效果", "渐变映射");
	f.registerNode<VintageEffect>("VintageEffect", "风格化与特效/色调效果", "复古/老照片");
	f.registerNode<EmbossEffect>("EmbossEffect", "风格化与特效/纹理效果", "浮雕效果");

	// 版面排版与装饰 - 排版
	f.registerNode<MultiImageLayout>("MultiImageLayout", "版面排版与装饰/排版", "多图排版");
	f.registerNode<CanvasExtend>("CanvasExtend", "版面排版与装饰/排版", "画布扩展");

	// 版面排版与装饰 - 装饰
	f.registerNode<BorderDecoration>("BorderDecoration", "版面排版与装饰/装饰", "边框与装饰");
		f.registerNode<ImageDiff>("ImageDiff", "版面排版与装饰/对比", "图像差异对比");
	f.registerNode<DropShadow>("DropShadow", "版面排版与装饰/装饰", "阴影效果");

	// 版面排版与装饰 - 显示
	f.registerNode<ImagePreview>("ImagePreview", "版面排版与装饰/显示", "图像预览显示");

	// 图形绘制与流程图 - 画布
	f.registerNode<BlankCanvas>("BlankCanvas", "图形绘制与流程图/画布", "空白画布创建");

	// 图形绘制与流程图 - 基础形状
	f.registerNode<DrawRectangle>("DrawRectangle", "图形绘制与流程图/基础形状", "矩形绘制");
	f.registerNode<DrawRoundedRect>("DrawRoundedRect", "图形绘制与流程图/基础形状", "圆角矩形绘制");
	f.registerNode<DrawEllipse>("DrawEllipse", "图形绘制与流程图/基础形状", "椭圆绘制");
	f.registerNode<DrawDiamond>("DrawDiamond", "图形绘制与流程图/基础形状", "菱形绘制");
	f.registerNode<DrawParallelogram>("DrawParallelogram", "图形绘制与流程图/基础形状", "平行四边形绘制");

	// 图形绘制与流程图 - 连接件
	f.registerNode<DrawArrow>("DrawArrow", "图形绘制与流程图/连接件", "直线/箭头绘制");
	f.registerNode<DrawPolyline>("DrawPolyline", "图形绘制与流程图/连接件", "折线连接绘制");
	f.registerNode<DrawCurve>("DrawCurve", "图形绘制与流程图/连接件", "曲线连接绘制");

	// 图形绘制与流程图 - 流程图
	f.registerNode<FlowchartElement>("FlowchartElement", "图形绘制与流程图/流程图", "流程图元素");
	f.registerNode<FlowchartAutoLayout>("FlowchartAutoLayout", "图形绘制与流程图/流程图", "流程图自动布局");

	// 图形绘制与流程图 - 装饰
	f.registerNode<IconEmbed>("IconEmbed", "图形绘制与流程图/装饰", "图标/符号嵌入");

	// 实用工具与高级 - 信息提取
	f.registerNode<ImageInfo>("ImageInfo", "实用工具与高级/信息提取", "图像信息提取");
	f.registerNode<ImageHistogramStats>("ImageHistogramStats", "实用工具与高级/信息提取", "图像直方图统计");
	f.registerNode<DataQualityScore>("DataQualityScore", "实用工具与高级/信息提取", "数据品质评分");

	// 实用工具与高级 - 流程控制
	f.registerNode<ConditionalRouter>("ConditionalRouter", "实用工具与高级/流程控制", "条件路由");
	f.registerNode<ListIterator>("ListIterator", "实用工具与高级/流程控制", "列表遍历器");

	f.registerNode<TableInfo>("TableInfo", "实用工具与高级/信息提取", "表格信息提取");
	f.registerNode<TextInfo>("TextInfo", "实用工具与高级/信息提取", "文本信息提取");
	f.registerNode<DataConditionalRouter>("DataConditionalRouter", "实用工具与高级/流程控制", "数据条件路由");
	// 实用工具与高级 - 外部接口
	f.registerNode<ExternalScript>("ExternalScript", "实用工具与高级/外部接口", "外部脚本执行");
	f.registerNode<ExternalAPICall>("ExternalAPICall", "实用工具与高级/外部接口", "外部API调用");

	// 几何图案生成
	f.registerNode<GenericPattern>("GenericPattern", "几何图案生成/通用图案", "通用几何图案");
	f.registerNode<CSPattern>("CSPattern", "几何图案生成/计算机科学", "计算机科学图案");
	f.registerNode<ChemistryPattern>("ChemistryPattern", "几何图案生成/化学", "化学图案");
	f.registerNode<BioPattern>("BioPattern", "几何图案生成/生命科学", "生命科学图案");
	f.registerNode<SocialSciPattern>("SocialSciPattern", "几何图案生成/社会科学", "社会科学图案");
	f.registerNode<CustomAssetImport>("CustomAssetImport", "几何图案生成/自定义素材", "自定义素材导入");
}
