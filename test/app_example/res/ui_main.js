var UI=require("gui2d/ui");
var W=require("gui2d/widgets");

UI.ChooseScalingFactor({designated_screen_size:360})
UI.Theme_Minimalistic([0xffcc7733])

var g_parameters={param0:0.3,param1:2};
UI.Application=function(id,attrs){
	UI.Begin(UI.Keep(id,attrs));
	var wnd=UI.Begin(W.Window('app',{
		title:'Example App',w:640,h:360,bgcolor:0xffffffff,
		flags:UI.SDL_WINDOW_MAXIMIZED|UI.SDL_WINDOW_RESIZABLE,
		property_sheet:{
			param0:[parseFloat(g_parameters.param0).toFixed(2),function(value){g_parameters.param0=value;}],
			param1_edit:[g_parameters.param1,function(value){g_parameters.param1=Math.max(Math.min(value,5),0);}],
			param1_slider:[g_parameters.param1/5,function(value){g_parameters.param1=Math.floor(value*5);}],
		},
		is_main_window:1}));
	/*widget*/(W.Label('text_363',{'x':254,'y':9,
		text:'Widget demo'}));
	/*widget*/(W.Label('text_435',{'x':12,'y':48.325592041015625,
		text:'EditBox'}));
	/*widget*/(W.EditBox('editbox_520',{'x':99.33997721652108,'y':48.325592041015625,'w':221.66002278347892,'h':24.325592041015625,
		}));
	/*widget*/(W.Label('text_683',{'x':355,'y':48.325592041015625,
		text:'Slider'}));
	/*widget*/(W.Slider('slider_767',{'x':421.67667382716087,'y':44.325592041015625,'w':203.92252664590268,'h':32,
		}));
	/*widget*/(W.Label('text_927',{'x':12,'y':84.65118408203125,
		text:'Select'}));
	/*widget*/(W.Select('select_1009',{'x':71.27753067016602,'y':80.65118408203125,'w':52.722469329833984,'h':32,
		items:[0,1]}));
	/*widget*/(W.Label('text_1185',{'x':145,'y':84.65118408203125,
		text:'Multi-ele'}));
	/*widget*/(W.Select('select_1276',{'x':230.52398032160067,'y':84.65118408203125,'w':168.01282620263464,'h':24.325592041015625,
		items:['A','B','C','D']}));
	/*widget*/(W.Label('text_1466',{'x':411.66079330444336,'y':84.65118408203125,
		text:'Long'}));
	/*widget*/(W.Select('select_1548',{'x':476.5638732910156,'y':84.65118408203125,'w':150.4286361641632,'h':24.325592041015625,
		items:["Item 0","Item 1","Long item"]}));
	/*widget*/(W.Label('text_1777',{'x':213.54624557495117,'y':121.97677612304688,
		text:'Complicated interactions'}));
	/*widget*/(W.Label('text_1895',{'x':12,'y':158.3023681640625,
		text:'Linked widgets'}));
	/*widget*/(W.EditBox('editbox_1986',{'x':172.91629791259766,'y':158.3023681640625,'w':52.176035373924655,'h':24.325592041015625,
		property_name:'param0'}));
	/*widget*/(W.Slider('slider_2151',{'x':237.8640373427824,'y':154.3023681640625,'w':386.039752062387,'h':32,
		property_name:'param0'}));
	/*widget*/(W.Button('button_2435',{'x':523.5144691958448,'y':236.5260978865856,'w':85.63090128755366,'h':29.10186231849255,
		property_name:'button_2435',
		text:'Quit',OnClick:function(){
			UI.DestroyWindow(UI.top.app)
		}}));
	/*widget*/(W.Label('text_2666',{'x':12,'y':194.62796020507813,
		text:'Integer slider'}));
	/*widget*/(W.EditBox('editbox_2758',{'x':171.57263673133076,'y':194.62796020507813,'w':53.519696555191544,'h':24.325592041015625,
		property_name:'param1_edit'}));
	/*widget*/(W.Slider('slider_2924',{'x':242.96810752784904,'y':190.62796020507813,'w':380.7930924701418,'h':32,
		property_name:'param1_slider'}));
	/*insert here*/
	UI.End();
	UI.End();
};
UI.Run()
