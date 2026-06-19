// nodes/drawing/FlowchartAutoLayout.h
#pragma once
#include "NodeBase.h"
#include <QColor>

class FlowchartAutoLayout : public NodeBase
{
public:
	FlowchartAutoLayout() = default;
	void defineNode() override;
	void process() override;

private:
	struct Node {
		QString id; QString name; QString logic; int x=0, y=0, w=0, h=0;
		int layer=0, order=0;
		QColor fill; QList<int> targets; QList<QString> labels;
	};
	static QString shapeForLogic(const QString& l);
	void layout(QVector<Node>& nodes);
	void drawShape(QPainter& p, const Node& n);
	QPoint exitPoint(const Node& n, int dir) const; // 0=下 1=右 2=上 3=左
	QPoint entryPoint(const Node& n, int dir) const;
};
