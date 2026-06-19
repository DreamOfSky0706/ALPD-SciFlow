// io/WorkflowSerializer.h
#pragma once

#include <QString>
#include <QFileInfo>

class WorkflowGraph;

class WorkflowSerializer
{
public:
	static bool save(WorkflowGraph* graph, const QString& filePath);
	static bool load(const QString& filePath, WorkflowGraph* graph);

	// canvas_view 传递（MainWindow在保存前设置）
	static void setCanvasView(double zoom, double cx, double cy);
	static void getCanvasView(double& zoom, double& cx, double& cy);
private:
	static double s_zoom, s_cx, s_cy;
};
