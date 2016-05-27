var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

UI.ChooseScalingFactor({designated_screen_size:480})
//choose a theme color
UI.Theme_Minimalistic(0xffb4771f)
//the main UI definition
UI.Application=function(id,attrs){
	UI.Begin(UI.Keep(id,attrs));
	var wnd=UI.Begin(W.Window('app',{
		title:'Example App',w:640,h:360,bgcolor:0xffffffff,
		flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
		is_main_window:1,
	}));
		UI.GLWidget(function(){CallLibFunction();})
	UI.End();
	UI.End();
};
UI.Run()
