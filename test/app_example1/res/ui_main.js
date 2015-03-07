var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

UI.ChooseScalingFactor({designated_screen_size:720})
UI.Theme_Minimalistic([0xff277fff])

UI.Application=function(id,attrs){
	UI.Begin(UI.Keep(id,attrs));
	var wnd=UI.Begin(W.Window('app',{
		title:'Example App',w:1280,h:720,bgcolor:0xffdddddd,
		flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
		property_sheet:{
		},
		is_main_window:1}));
	var obj=W.AutoHidePanel("panel0",{
		x:0,y:0,w:300,anchor_placement:'right',knob_size:20,
	})
	UI.Begin(obj)
		W.RoundRect("",{anchor:'parent',anchor_align:'fill',anchor_valign:'fill',x:0,y:0,
			color:0xff00ff00,border_width:0})
	UI.End()
	/*insert here*/
	UI.End();
	UI.End();
};
UI.Run()
