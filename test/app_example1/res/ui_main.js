var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

UI.ChooseScalingFactor({designated_screen_size:720})
UI.Theme_Minimalistic([0xff277fff])

UI.Application=function(id,attrs){
	UI.Begin(UI.Keep(id,attrs));
	var wnd=UI.Begin(W.Window('app',{
		title:'Example App',w:1280,h:720,bgcolor:0xffffffff,
		flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
		property_sheet:{
			param0:[parseFloat(g_parameters.param0).toFixed(2),function(value){g_parameters.param0=value;}],
			param1_edit:[g_parameters.param1,function(value){g_parameters.param1=Math.max(Math.min(value,5),0);}],
			param1_slider:[g_parameters.param1/5,function(value){g_parameters.param1=Math.floor(value*5);}],
		},
		is_main_window:1}));
	!? //W.AutoHidePanel
	/*insert here*/
	UI.End();
	UI.End();
};
UI.Run()
