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
UI.default_styles.button={\n\
	transition_dt:0.1,\n\
	round:24,border_width:3,padding:12,\n\
	$:{\n\
		out:{\n\
			border_color:0xffcc773f,color:0xffffffff,\n\
			icon_color:0xffcc773f,\n\
			text_color:0xffcc773f,\n\
		},\n\
		over:{\n\
			border_color:0xffcc773f,color:0xffcc773f,\n\
			icon_color:0xffffffff,\n\
			text_color:0xffffffff,\n\
		},\n\
		down:{\n\
			border_color:0xffaa5522,color:0xffaa5522,\n\
			icon_color:0xffffffff,\n\
			text_color:0xffffffff,\n\
		},\n\
	}\n\
};\n\
\n\
UI.Application=function(id,attrs){\n\
	attrs=UI.Keep(id,attrs);\n\
	UI.Begin(attrs);\n\
		var wnd=UI.Begin(W.Window('app',{\n\
						title:'Jacy test code',w:1280,h:720,bgcolor:0xffffffff,\n\
						designated_screen_size:1440,flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,\n\
						is_main_window:1}));\n\
			W.Text('',/*widget*/({\n\
				anchor:UI.context_parent,anchor_align:'left',anchor_valign:'up',\n\
				w:UI.context_parent.w-32,\n\
				x:16,y:16,\n\
				font:UI.Font('msyh',128,-100),text:'标题很细',\n\
				color:0xff000000,\n\
				}));\n\
			W.Button('ok',/*widget*/({\n\
				anchor:UI.context_parent,anchor_align:'right',anchor_valign:'down',\n\
				x:16,y:16,\n\
				font:UI.Font('ArialUni',48),text:'OK',\n\
				OnClick:function(){UI.DestroyWindow(wnd)}}));\n\
		UI.End();\n\
	UI.End();\n\
};\n\
";
var item_0={id:"$0",x:10,y:10,w:400,h:300,w_min:50,h_min:50,OnChange:function(attrs){item_0.x=attrs.x;item_0.y=attrs.y;item_0.w=attrs.w;item_0.h=attrs.h}};
var item_1={id:"$1",x:20,y:20,w:200,h:200,w_min:50,h_min:50,OnChange:function(attrs){item_1.x=attrs.x;item_1.y=attrs.y;item_1.w=attrs.w;item_1.h=attrs.h}};
var g_language_C=Language.Define(function(lang){
	var bid_comment=lang.ColoredDelimiter("key","/*","*/","color_comment");
	var bid_comment2=lang.ColoredDelimiter("key","//","\n","color_comment");
	var bid_string=lang.ColoredDelimiter("key",'"','"',"color_string");
	var bid_string2=lang.ColoredDelimiter("key","'","'","color_string");
	var bid_bracket=lang.DefineDelimiter("nested",['(','[','{'],['}',']',')']);
	lang.DefineToken("\\\\")
	lang.DefineToken("\\'")
	lang.DefineToken('\\"')
	lang.DefineToken('\\\n')
	return (function(lang){
		lang.SetExclusive([bid_comment,bid_comment2,bid_string,bid_string2]);
		if(lang.isInside(bid_comment)||lang.isInside(bid_comment2)||lang.isInside(bid_string)||lang.isInside(bid_string2)){
			lang.Disable(bid_bracket);
		}else{
			lang.Enable(bid_bracket);
		}
	});
});
//todo

var UpdateWorkingCode=function(){
	var code_box=UI.top.app.code_box;
	var ed=code_box.ed;
	var ccnt_sel=code_box.sel1.ccnt;
	var range_0=code_box.FindOuterBracket(ccnt_sel,-1);
	var range_1=code_box.FindOuterBracket(ccnt_sel,1);
	print(ed.GetText(range_0,range_1-range_0))
	//todo: reset global environment?
};

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
			var ed_box=W.Edit("code_box",{
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
				OnSelectionChange:function(){
					UpdateWorkingCode();
				},
				///////////////
				x:0,y:0,w:ed_rect.w-8,h:ed_rect.h-8,
			});
			//this part is effectively a GLwidget
			//todo: clipping
			UI.GLWidget(function(){g_sandbox.DrawWindow(16,16);})
			//todo: /*widget*/ hacks - for the current, search for bracket and parse
			//for general evaling, just replace
			//global styles
			W.Group("controls",{'item_object':W.BoxDocumentItem,'items':[item_0,item_1]})
		UI.End();
		///////////////////
		//this calls BeginPaint which is not reentrant... consider it as a separate window
		var s_code=ed_box.ed.GetText();
		//g_sandbox.eval("UI.Application=function(id,attrs){"+s_code+"};UI.DrawFrame();");
	UI.End();
};
UI.Run()
