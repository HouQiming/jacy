var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

var DrawTabs=function(min_size,layout,x,y,w,h){
	var dc,dc0;
	if(layout.direction=='x'){
		dc=w;
	}else if(layout.direction=='y'){
		dc=h;
	}else if(layout.direction=='tab'){
		//todo: tabbar drawing, tab dragging (could do it later)
		if(!layout.items.length){return;}
		if(!layout.active_tab){layout.active_tab=0;}
		DrawTabs(min_size,layout.items[layout.active_tab],x,y,w,h);
		return;
	}else{
		layout.object_type(layout);
		return;
	}
	if(layout.ratio){
		//todo: sizing
		dc0=dc*layout.ratio;
	}else{
		//allow absolute layouts, but do not produce them while dragging
		dc0=layout.absolute_offset;
		if(dc0<0){dc0+=dc;}
		if(dc0>=dc-min_size||dc0<=min_size){
			dc0=dc*0.5;
		}
	}
	if(layout.direction=='x'){
		DrawTabs(min_size,layout[0],x,y,dc0,h);
		DrawTabs(min_size,layout[1],x+dc0,y,dc-dc0,h);
	}else{
		DrawTabs(min_size,layout[0],x,y,w,dc0);
		DrawTabs(min_size,layout[1],x,y+dc0,w,h-dc0);
	}
};
W.DockingLayout_prototype={layout_direction:"inside"}
W.DockingLayout=function(id,attrs){
	var obj=UI.Keep(id,attrs,W.DockingLayout_prototype);
	UI.StdAnchoring(id,obj);
	/////////
	DrawTabs(obj.min_size||100,obj.layout,obj.x,obj.y,obj.w,obj.h)
	return obj;
};
