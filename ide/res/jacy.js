var UI=require("gui2d/ui");
var W=require("gui2d/widgets");
require("res/lib/boxdoc");
var Language=require("res/lib/langdef");

function parent(){return UI.context_parent;}

UI.SetFontSharpening(1.5)
var g_sandbox=UI.CreateSandbox();
g_sandbox.eval("var UI=require('gui2d/ui');var W=require('gui2d/widgets');")
//todo
g_sandbox.m_relative_scaling=0.5;
var g_initial_code="\
/* Test comment string */\n\
attrs=UI.Keep(id,attrs);\n\
UI.Begin(attrs);\n\
	var wnd=UI.Begin(W.Window('app',{\n\
					title:'Jacy test code',w:1280,h:720,bgcolor:0xffffffff,\n\
					designated_screen_size:1440,flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,\n\
					is_main_window:1}));\n\
		W.Text('',{\n\
			anchor:UI.context_parent,anchor_align:'left',anchor_valign:'up',\n\
			w:UI.context_parent.w-32,\n\
			x:16,y:16,\n\
			font:UI.Font('msyh',128,-100),text:'标题很细',\n\
			color:0xff000000,\n\
			});\n\
	UI.End();\n\
UI.End();\n\
";
var item_0={id:"$0",x:10,y:10,w:400,h:300,w_min:50,h_min:50,OnChange:function(attrs){item_0.x=attrs.x;item_0.y=attrs.y;item_0.w=attrs.w;item_0.h=attrs.h}};
var item_1={id:"$1",x:20,y:20,w:200,h:200,w_min:50,h_min:50,OnChange:function(attrs){item_1.x=attrs.x;item_1.y=attrs.y;item_1.w=attrs.w;item_1.h=attrs.h}};
var g_language_C=Language.Define(function(lang){
	var bid_comment=lang.ColoredDelimiter("key","/*","*/","color_comment");
	var bid_comment2=lang.ColoredDelimiter("key","//","\n","color_comment");
	var bid_string=lang.ColoredDelimiter("key",'"','"',"color_string");
	var bid_string2=lang.ColoredDelimiter("key","'","'","color_string");
	lang.DefineToken("\\\\")
	lang.DefineToken("\\'")
	lang.DefineToken('\\"')
	lang.DefineToken('\\\n')
	var global_bids=[bid_comment,bid_comment2,bid_string,bid_string2];
	return (function(lang){
		for(var i=0;i<global_bids.length;i++){
			lang.Enable(global_bids[i]);
		}
		for(var i=0;i<global_bids.length;i++){
			if(lang.isInside(global_bids[i])){
				for(var j=0;j<global_bids.length;j++){
					lang.Disable(global_bids[j]);
				}
				lang.Enable(global_bids[i]);
				break;
			}
		}
	});
});
//todo

UI.Application=function(id,attrs){
	attrs=UI.Keep(id,attrs);
	UI.Begin(attrs);
		///////////////////
		var wnd=UI.Begin(W.Window('app',{
				title:'Jacy IDE',w:1280,h:720,bgcolor:0xffbbbbbb,
				designated_screen_size:1440,flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
				is_main_window:1}));
			if(UI.Platform.ARCH!="mac"&&UI.Platform.ARCH!="ios"){
				W.Hotkey("",{key:["ALT","F4"],action:function(){UI.DestroyWindow(wnd)}});
			}
			var ed_rect=W.RoundRect("",{
				color:0xffffffff,border_color:0xff444444,border_width:2,
				anchor:parent(),anchor_align:"right",anchor_valign:"center",
				x:16,y:0,w:wnd.w*0.3,h:wnd.h-32,
			});
			var ed_box=W.Edit("textbox",{
				font:UI.Font("res/fonts/inconsolata.ttf",32),color:0xff000000,
				tab_width:4,
				text:g_initial_code,//todo
				anchor:ed_rect,anchor_align:"center",anchor_valign:"center",
				///////////////
				state_handlers:["renderer_programmer","colorer_programmer"],
				language:g_language_C,
				color_string:0xff0055aa,
				color_comment:0xff008000,
				///////////////
				x:0,y:0,w:ed_rect.w-8,h:ed_rect.h-8,
			});
			//this part is effectively a GLwidget
			//todo: clipping
			UI.GLWidget(function(){g_sandbox.DrawWindow(16,16);})
			W.Group("controls",{'item_object':W.BoxDocumentItem,'items':[item_0,item_1]})
		UI.End();
		///////////////////
		//this calls BeginPaint which is not reentrant... consider it as a separate window
		var s_code=ed_box.ed.GetText();
		g_sandbox.eval("UI.Application=function(id,attrs){"+s_code+"};UI.DrawFrame();");
	UI.End();
};
UI.Run()
