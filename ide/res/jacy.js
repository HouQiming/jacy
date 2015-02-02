var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

function parent(){return UI.context_parent;}
var g_sandbox=UI.CreateSandbox();
g_sandbox.eval("var UI=require('gui2d/ui');var W=require('gui2d/widgets');")
//todo
g_sandbox.m_relative_scaling=0.5;
var g_initial_code="\
	attrs=UI.Keep(id,attrs);\n\
	UI.Begin(attrs);\n\
		var wnd=UI.Begin(W.Window('app',{\n\
						title:'Jacy test code',w:1280,h:720,bgcolor:0xff008000,\n\
						designated_screen_size:1440,flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,\n\
						is_main_window:1}));\n\
		UI.End();\n\
	UI.End();\n\
";
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
				text:g_initial_code,//todo
				anchor:ed_rect,anchor_align:"center",anchor_valign:"center",
				x:0,y:0,w:ed_rect.w-8,h:ed_rect.h-8,
			});
			//this part is effectively a GLwidget
			UI.AddGLCall(function(){g_sandbox.DrawWindow(16,16);})
		UI.End();
		///////////////////
		//this calls BeginPaint which is not reentrant... consider it as a separate window
		var s_code=ed_box.ed.GetText();
		g_sandbox.eval("UI.Application=function(id,attrs){"+s_code+"};UI.DrawFrame();");
	UI.End();
};
UI.Run()
