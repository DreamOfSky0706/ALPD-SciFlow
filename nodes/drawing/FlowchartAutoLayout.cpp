// nodes/drawing/FlowchartAutoLayout.cpp
#include "FlowchartAutoLayout.h"
#include "Utility.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QFontMetrics>
#include <QMap>
#include <QQueue>
#include <algorithm>
#include <cmath>

void FlowchartAutoLayout::defineNode()
{
	addInputPort("input_data", DataType::DataTable, false);
	addOutputPort("output_image", DataType::Image);
	addParam("steps", "步骤定义", ParamType::FlowchartSteps, QVariantList());
	addParam("color_scheme", "配色方案", ParamType::Combo, QString("科技蓝"),
		{{"options",QStringList{"科技蓝","学术灰","清新绿","暖色调","商务深蓝"}}});
	addParam("direction", "方向", ParamType::Combo, QString("自上而下"),
		{{"options",QStringList{"自上而下","自左而右"}}});
	addParam("font_size", "字号", ParamType::IntSlider, 13, {{"min",8},{"max",36}});
	addParam("canvas_width","画布宽",ParamType::Int,1600,{{"min",400},{"max",8000}});
	addParam("canvas_height","画布高",ParamType::Int,1200,{{"min",300},{"max",8000}});
	addParam("node_width","节点宽",ParamType::Int,140,{{"min",60},{"max",400}});
	addParam("node_height","节点高",ParamType::Int,50,{{"min",30},{"max",200}});
	addParam("h_gap","水平间距",ParamType::Int,80,{{"min",20},{"max",300}});
	addParam("v_gap","垂直间距",ParamType::Int,60,{{"min",20},{"max",300}});
}

QString FlowchartAutoLayout::shapeForLogic(const QString& l)
{
	if(l=="Start"||l=="开始"||l=="End"||l=="结束") return "ellipse";
	if(l=="Decision"||l=="判断/分支") return "diamond";
	if(l=="I-O"||l=="输入/输出") return "parallelogram";
	return "roundrect";
}

QPoint FlowchartAutoLayout::entryPoint(const Node& n, int dir) const
{
	int cx=n.x+n.w/2, cy=n.y+n.h/2;
	if(dir==0) return {cx, n.y};        // 上
	if(dir==1) return {n.x+n.w, cy};    // 右
	if(dir==2) return {cx, n.y+n.h};    // 下
	return {n.x, cy};                    // 左
}

QPoint FlowchartAutoLayout::exitPoint(const Node& n, int dir) const
{
	int cx=n.x+n.w/2, cy=n.y+n.h/2;
	if(dir==0) return {cx, n.y+n.h};    // 下
	if(dir==1) return {n.x+n.w, cy};    // 右
	if(dir==2) return {cx, n.y};        // 上
	return {n.x, cy};                    // 左
}

void FlowchartAutoLayout::layout(QVector<Node>& nodes)
{
	if(nodes.isEmpty()) return;
	int N=nodes.size();
	// 构建邻接表 + 入度
	QVector<QList<int>> adj(N);
	QVector<int> inDeg(N,0);
	QVector<bool> isBackEdge(N*N, false);
	for(int i=0;i<N;++i)
		for(int t:nodes[i].targets)
			if(t>=0 && t<N){ adj[i].append(t); inDeg[t]++; }
	// Kahn分层
	QQueue<int> q;
	QVector<int> layer(N,0);
	for(int i=0;i<N;++i) if(inDeg[i]==0) q.enqueue(i);
	while(!q.isEmpty()){
		int u=q.dequeue();
		for(int v:adj[u]){
			layer[v]=qMax(layer[v],layer[u]+1);
			if(--inDeg[v]==0) q.enqueue(v);
		}
	}
	// 回边节点放到最大层+1
	int maxL=0; for(int l:layer) maxL=qMax(maxL,l);
	for(int i=0;i<N;++i) if(inDeg[i]>0) layer[i]=++maxL;

	// 按层分组
	QVector<QList<int>> layerNodes(maxL+1);
	for(int i=0;i<N;++i) layerNodes[layer[i]].append(i);

	bool vert=(param("direction").toString()=="自上而下");
	int nw=param("node_width").toInt(), nh=param("node_height").toInt();
	int hg=param("h_gap").toInt(), vg=param("v_gap").toInt();

	// 分配坐标
	int curX=50, curY=50;
	for(int l=0;l<=maxL;++l){
		auto& list=layerNodes[l];
		if(list.isEmpty()) continue;
		int colY=curY;
		for(int idx:list){
			nodes[idx].w=nw; nodes[idx].h=nh;
			nodes[idx].x=curX; nodes[idx].y=colY;
			nodes[idx].layer=l;
			if(vert) colY+=nh+vg;
			else colY+=nw+hg;
		}
		if(vert) curX+=nw+hg;
		else curY+=nh+vg;
	}
}

void FlowchartAutoLayout::drawShape(QPainter& p, const Node& n)
{
	QString s=shapeForLogic(n.logic);
	p.setBrush(n.fill);
	p.setPen(QPen(n.fill.darker(150),2));
	int x=n.x, y=n.y, w=n.w, h=n.h;
	if(s=="ellipse"){
		p.drawEllipse(x,y,w,h);
	}else if(s=="diamond"){
		QPolygon poly; poly<<QPoint(x+w/2,y)<<QPoint(x+w,y+h/2)<<QPoint(x+w/2,y+h)<<QPoint(x,y+h/2);
		p.drawPolygon(poly);
	}else if(s=="parallelogram"){
		int sk=w/5;
		QPolygon poly; poly<<QPoint(x+sk,y)<<QPoint(x+w,y)<<QPoint(x+w-sk,y+h)<<QPoint(x,y+h);
		p.drawPolygon(poly);
	}else{
		p.drawRoundedRect(x,y,w,h,8,8);
	}
	// 文字
	p.setPen(QColor(0,0,0));
	QFont f; f.setPointSize(param("font_size").toInt()); p.setFont(f);
	p.drawText(QRect(x,y,w,h), Qt::AlignCenter, n.name);
}

void FlowchartAutoLayout::process()
{
	QVariantList sd=param("steps").toList();
	if(sd.isEmpty()){ reportError("步骤列表为空"); return; }

	int cw=param("canvas_width").toInt(), ch=param("canvas_height").toInt();
	QString scheme=param("color_scheme").toString();
	bool vert=(param("direction").toString()=="自上而下");
	int fs=param("font_size").toInt();

	// 配色
	QColor fillDefault(220,235,255), fillStart(180,220,240), fillEnd(220,200,200),
	       fillDecision(255,240,210), fillIO(240,225,255);
	if(scheme=="学术灰"){ fillDefault=QColor(230,230,230); fillStart=QColor(200,210,220); fillEnd=QColor(210,200,200); fillDecision=QColor(240,235,225); fillIO=QColor(225,220,235); }
	else if(scheme=="清新绿"){ fillDefault=QColor(210,245,220); fillStart=QColor(180,225,200); fillEnd=QColor(225,210,210); fillDecision=QColor(255,250,215); fillIO=QColor(230,215,250); }
	else if(scheme=="暖色调"){ fillDefault=QColor(255,240,220); fillStart=QColor(240,220,200); fillEnd=QColor(240,200,200); fillDecision=QColor(255,225,200); fillIO=QColor(245,220,255); }
	else if(scheme=="商务深蓝"){ fillDefault=QColor(210,225,245); fillStart=QColor(180,200,230); fillEnd=QColor(220,200,210); fillDecision=QColor(245,235,210); fillIO=QColor(230,215,245); }

	// 构建nodes
	QVector<Node> nodes; QMap<QString,int> idMap;
	for(int i=0;i<sd.size();++i){
		QVariantMap m=sd[i].toMap();
		Node nd; nd.id=m.value("step_id",QString("s%1").arg(i)).toString();
		nd.name=m.value("step_name","").toString();
		nd.logic=m.value("logic_type","Process").toString();
		idMap[nd.id]=i;
		// 自动颜色
		if(nd.logic=="Start"||nd.logic=="开始") nd.fill=fillStart;
		else if(nd.logic=="End"||nd.logic=="结束") nd.fill=fillEnd;
		else if(nd.logic=="Decision"||nd.logic=="判断/分支") nd.fill=fillDecision;
		else if(nd.logic=="I-O"||nd.logic=="输入/输出") nd.fill=fillIO;
		else nd.fill=fillDefault;
		nodes.append(nd);
	}
	// 连线
	for(int i=0;i<sd.size();++i){
		QVariantMap m=sd[i].toMap();
		QVariantList conns=m.value("connections").toList();
		for(const auto& c:conns){
			QVariantMap cm=c.toMap();
			QString tid=cm.value("target").toString();
			if(idMap.contains(tid)){
				nodes[i].targets.append(idMap[tid]);
				nodes[i].labels.append(cm.value("label","").toString());
			}
		}
		// 判断分支
		if(nodes[i].logic=="Decision"||nodes[i].logic=="判断/分支"){
			int bt=m.value("branch_true",-1).toInt();
			int bf=m.value("branch_false",-1).toInt();
			if(bt>=0&&bt<sd.size()){ nodes[i].targets.append(bt); nodes[i].labels.append("是"); }
			if(bf>=0&&bf<sd.size()){ nodes[i].targets.append(bf); nodes[i].labels.append("否"); }
		}
	}

	layout(nodes);

	// 绘制
	QImage img(cw,ch,QImage::Format_ARGB32);
	img.fill(Qt::white);
	QPainter p(&img); p.setRenderHint(QPainter::Antialiasing);

	// 连线（使用正交折线）
	QPen linePen(QColor(60,60,60),2);
	for(int i=0;i<nodes.size();++i){
		const auto& src=nodes[i];
		for(int j=0;j<src.targets.size();++j){
			int tid=src.targets[j];
			if(tid<0||tid>=nodes.size()) continue;
			const auto& dst=nodes[tid];
			bool isDecision=(src.logic=="Decision"||src.logic=="判断/分支");
			bool isNo=(src.labels.value(j,"")=="否");

			QPoint from, to;
			QPainterPath path;
			if(isDecision && isNo){
				// 否分支：从右侧出→向右弯→从左侧入目标
				from=exitPoint(src,1); // 右出
				to=entryPoint(dst,3);  // 左入
				int midX=qMax(from.x(),to.x())+40;
				path.moveTo(from);
				path.lineTo(midX, from.y());
				path.lineTo(midX, to.y());
				path.lineTo(to);
			}else{
				// 正常分支：从下出→从上入
				from=exitPoint(src,0); to=entryPoint(dst,0);
				int midY=(from.y()+to.y())/2;
				path.moveTo(from);
				path.lineTo(from.x(), midY);
				path.lineTo(to.x(), midY);
				path.lineTo(to);
			}
			p.setPen(linePen); p.setBrush(Qt::NoBrush);
			p.drawPath(path);

			// 箭头
			QPointF ep=path.currentPosition();
			QPointF prev=path.pointAtPercent(0.95);
			double dx=ep.x()-prev.x(), dy=ep.y()-prev.y();
			double len=std::sqrt(dx*dx+dy*dy);
			if(len>0){ dx/=len; dy/=len; }
			double as=8;
			QPolygonF head;
			head<<ep<<QPointF(ep.x()-as*dx+as*0.4*dy, ep.y()-as*dy-as*0.4*dx)
			    <<QPointF(ep.x()-as*dx-as*0.4*dy, ep.y()-as*dy+as*0.4*dx);
			p.setPen(Qt::NoPen); p.setBrush(QColor(60,60,60));
			p.drawPolygon(head);

			// 标签
			QString lbl=src.labels.value(j,"");
			if(!lbl.isEmpty()){
				QPointF mid=path.pointAtPercent(0.5);
				p.setPen(QColor(80,80,80));
				QFont lf; lf.setPointSize(fs-2); p.setFont(lf);
				p.drawText(QRectF(mid.x()-20,mid.y()-14,40,14),Qt::AlignCenter,lbl);
			}
		}
	}

	// 画节点
	for(const auto& nd:nodes) drawShape(p,nd);

	p.end();
	cv::Mat dst=Utility::qImageToMat(img);
	setOutput("output_image",NodeData::createImage(dst));
}
