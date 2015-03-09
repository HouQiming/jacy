var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

UI.ChooseScalingFactor({designated_screen_size:960})
UI.Theme_Minimalistic([0xff277fff])

var g_params={a:100,b:50};

var PortraitPanel=function(id,attrs){
	/*editor: UI.BeginVirtualWindow(id,{w:480,h:300,bgcolor:0xffffffff})//*/
	W.RoundRect("",{anchor:'parent',anchor_align:'fill',anchor_valign:'fill',x:0,y:0,
		color:0xcc000000,border_width:0});
	/*widget*/(W.Label('text_543',{'x':33,'y':19,
		text:'Param A',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_629',{'x':111.86343383789063,'y':19,'w':94.91212600621634,'h':24.471364974975586,
		property_name:'a'}));
	/*widget*/(W.Label('text_544',{'x':33,'y':61,
		text:'Param B',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_639',{'x':111.86343383789063,'y':61,'w':94.91212600621634,'h':24.471364974975586,
		property_name:'b'}));
	/*widget*/(W.Label('text_999',{'x':265.42439490764264,'y':43.471364974975586,
		text:'Result',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_1098',{'x':343.6791545461065,'y':43.471364974975586,'w':95.08835502813736,'h':24.471364974975586,
		property_name:'result'}));
	/*widget*/(W.Label('text_999x',{'x':33.60867419942679,'y':103.47136497497559,
		text:'SHA1',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_1098x',{'x':111.86343383789063,'y':103.47136497497559,'w':326.90407573635326,'h':24.471364974975586,
		property_name:'result_SHA1'}));
	/*insert here*/
	/*editor: UI.EndVirtualWindow()//*/
}

var PortraitMode=function(id,attrs){
	/*editor: UI.BeginVirtualWindow(id,{w:480,h:720,bgcolor:0xffffffff})//*/
	var obj=W.AutoHidePanel("panel",{
		x:0,y:0,h:300,anchor_placement:'down',knob_size:100,
		/*editor: position:300,//*/
	});
	/*widget*/(W.Label('text_2435',{
		anchor:obj,anchor_placement:'up',anchor_align:'center',anchor_valign:'down',
		'x':0,'y':81.58590698242188,
		font:UI.Font(UI.font_name,72,-50),
		text:'Pull here',color:UI.current_theme_color}));
	/*widget*/(W.Label('text_2435',{
		anchor:obj,anchor_placement:'up',anchor_align:'center',anchor_valign:'down',
		'x':0,'y':0,
		font:UI.Font(UI.font_name,96,-50),
		text:'\u2191',color:UI.current_theme_color}));
	/*insert here*/
	UI.Begin(obj)
	PortraitPanel()
	UI.End()
	/*editor: UI.EndVirtualWindow()//*/
}

var LandscapePanel=function(id,attrs){
	/*editor: UI.BeginVirtualWindow(id,{w:300,h:480,bgcolor:0xffffffff})//*/
	W.RoundRect("",{anchor:'parent',anchor_align:'fill',anchor_valign:'fill',x:0,y:0,
		color:0xcc000000,border_width:0});
	/*widget*/(W.Label('text_543',{'x':13,'y':22,
		text:'Param A',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_629',{'x':91.86343383789063,'y':22,'w':186.38531251363247,'h':24.471364974975586,
		property_name:'a'}));
	/*widget*/(W.Label('text_544',{'x':13,'y':64,
		text:'Param B',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_639',{'x':91.86343383789063,'y':64,'w':186.38531251363247,'h':24.471364974975586,
		property_name:'b'}));
	/*widget*/(W.Label('text_999',{'x':13.608674199426787,'y':105.4713649749756,
		text:'Result',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_1098',{'x':90.55325717579032,'y':105.4713649749756,'w':187.701687038509,'h':24.471364974975586,
		property_name:'result'}));
	/*widget*/(W.Label('text_999x',{'x':221.02407594136685,'y':184.65134930105953,
		text:'SHA1 \u2191',color:0xffffffff}));
	/*widget*/(W.EditBox('editbox_1098x',{'x':13,'y':141.94272994995117,'w':265.25809823585763,'h':30.708619351108347,
		property_name:'result_SHA1'}));
	/*insert here*/
	/*editor: UI.EndVirtualWindow()//*/
}

var LandscapeMode=function(id,attrs){
	/*editor: UI.BeginVirtualWindow(id,{w:720,h:480,bgcolor:0xffffffff})//*/
	var obj=W.AutoHidePanel("panel",{
		x:0,y:0,w:300,anchor_placement:'right',knob_size:100,
		/*editor: position:300,//*/
	});
	/*widget*/(W.Label('text_2435',{
		anchor:obj,anchor_placement:'left',anchor_align:'right',anchor_valign:'center',
		'x':66.32084545389353,'y':0,
		font:UI.Font(UI.font_name,72,-50),
		text:'Pull here',color:UI.current_theme_color}));
	/*widget*/(W.Label('text_2435',{
		anchor:obj,anchor_placement:'left',anchor_align:'right',anchor_valign:'center',
		'x':1,'y':-4,
		font:UI.Font(UI.font_name,96,-50),
		text:'\u2190',color:UI.current_theme_color}));
	UI.Begin(obj)
	LandscapePanel()
	UI.End()
	/*editor: UI.EndVirtualWindow()//*/
}

UI.Application=function(id,attrs){
	RunNativeCode();
	UI.Begin(UI.Keep(id,attrs));
	var wnd=UI.Begin(W.Window('app',{
		title:'Example App 1',w:1280,h:720,bgcolor:0xffdddddd,
		flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
		is_main_window:1,
		property_sheet:{
			a:[g_params.a,function(value){g_params.a=parseFloat(value)}],
			b:[g_params.b,function(value){g_params.b=parseFloat(value)}],
			result:[g_params.result],
			result_SHA1:[NativeFunction(g_params.result)],
		},
	}));
	if(wnd.w<wnd.h){
		PortraitMode(id,attrs);
	}else{
		LandscapeMode(id,attrs);
	}
	UI.End();
	UI.End();
};
UI.Run()
