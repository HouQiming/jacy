////////////////////////////////////////
//basic primitives
var UI=require("gui2d/ui");
var W=exports;

UI.font_name="LucidaGrande,_H_HelveticaNeue,segoeui,Roboto-Regular,Arial";
UI.font_size=24;
//UI.SetSRGBEnabling(2);
UI.SetFontSharpening(1);
UI.Theme_Minimalistic=function(C){
	UI.current_theme_color=C;
	var C_dark=UI.lerp_rgba(C,0xff000000,0.15)
	var C_sel=UI.lerp_rgba(C,0xffffffff,0.75)
	UI.default_styles={
		"label":{
			font:UI.Font(UI.font_name,UI.font_size),
			color:0xff000000,
		},
		tooltip:{
			font:UI.Font(UI.font_name,UI.font_size),
			padding:8,
			spacing:8,
			color:0xffffffff,
			round:8,
			border_color:0xff000000,
			border_width:1,
			text_color:0xff000000,
			shadow_size:6,
			shadow_color:0xaa000000,
			triangle_font:UI.Font(UI.font_name,32,0),
			triangle_font2:UI.Font(UI.font_name,32,250),
		},
		button:{
			transition_dt:0.1,
			round:0,border_width:1,padding:4,
			font:UI.Font(UI.font_name,UI.font_size),
			$:{
				out:{
					//border_color:0xff444444,color:0xffffffff,
					border_color:0xff000000,color:[{x:0,y:0,color:0xffffffff},{x:0,y:1,color:0xffe8e8e8}],
					icon_color:0xff000000,
					text_color:0xff000000,
				},
				over:{
					border_color:C,color:C,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				down:{
					border_color:C_dark,color:C_dark,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				checked_out:{
					border_color:0xff444444,color:0xffe8e8e8,
					icon_color:0xff7f7f7f,
					text_color:0xff7f7f7f,
				},
				checked_over:{
					border_color:0xff444444,color:0xffe8e8e8,
					icon_color:0xff7f7f7f,
					text_color:0xff7f7f7f,
				},
				checked_down:{
					border_color:0xff444444,color:0xffe8e8e8,
					icon_color:0xff7f7f7f,
					text_color:0xff7f7f7f,
				},
			}
		},
		tool_button:{
			transition_dt:0.1,
			round:0,border_width:3,padding:12,
			$:{
				out:{
					border_color:0x00ffffff,color:0x00ffffff,
					icon_color:C,
					text_color:C,
				},
				over:{
					border_color:C,color:C,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				down:{
					border_color:C_dark,color:C_dark,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
			}
		},
		check_button:{
			transition_dt:0.1,
			round:0,border_width:2,padding:12,
			$:{
				out:{
					border_color:C&0x00ffffff,color:0x00ffffff,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
				over:{
					border_color:C,color:0x00ffffff,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
				down:{
					border_color:C_dark,color:0x00ffffff,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
				////////////////////
				checked_out:{
					border_color:C&0x00ffffff,color:C_sel,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
				checked_over:{
					border_color:C,color:C_sel,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
				checked_down:{
					border_color:C_dark,color:C_sel,
					icon_color:0xff444444,
					text_color:0xff444444,
				},
			}
		},
		edit:{
			//animating edit would ruin a lot of object properties
			transition_dt:0,
			scroll_transition_dt:0.1,
			bgcolor_selection:C_sel,
			caret_width:UI.IS_MOBILE?1:2,
			caret_color:0xff000000,
			caret_flicker:500,
		},
		menu_item:{
			font:UI.Font(UI.font_name,UI.font_size),
			transition_dt:0.1,
			round:0,padding:8,
			icon_color:0xff000000,
			text_color:0xff000000,
			color:0x00ffffff,
			$:{
				over:{
					color:C,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
			},
		},
		menu:{
			transition_dt:0.1,
			round:4,border_width:2,padding:8,
			layout_spacing:0,
			border_color:C,color:0xffffffff,
		},
		combobox:{
			transition_dt:0.1,
			round:4,border_width:2,padding:8,
			layout_spacing:0,
			border_color:C,icon_color:C,text_color:0xff000000,color:0xffffffff,icon_text_align:'left',
			font:UI.Font(UI.font_name,UI.font_size),
			label_font:UI.Font(UI.font_name,UI.font_size),
		},
		slider:{
			transition_dt:0.1,
			bgcolor:[{x:0,y:0,color:0xffbbbbbb},{x:0,y:1,color:0xffdddddd}],
			//border_width:2, border_color:0xff444444,
			h_slider:8,
			round:8,
			color:C,
			padding:0,
			//label_text:'▲',
			//label_raise:0.4,
			//label_font:UI.Font(UI.font_name,32),
			//label_color:C,
			middle_bar:{
				w:8,h:8,
				round:2,
				color:0xffffffff, border_width:2, border_color:0xff444444,
			},
		},
		scroll_bar:{
			transition_dt:0.1,
			bgcolor:0,
			round:8,
			padding:0,
			szbar_min:32,
			middle_bar:{
				w:8,h:8,
				round:4,
				color:C, border_color:0,
			},
			$:{
				out:{},
				over:{},
			},
		},
		list_view:{
			color:0,border_color:0,
			size_scroll_bar:8,
			has_scroll_bar:!UI.IS_MOBILE,
		},
		edit_box:{
			transition_dt:0.1,
			round:4,padding:8,
			color:0xffffffff,
			hint_color:0xffaaaaaa,
			border_width:2,
			border_color:0xffaaaaaa,
			font:UI.Font(UI.font_name,UI.font_size),
			text_color:0xff000000,
			$:{
				blur:{
					border_color:0xffaaaaaa,
				},
				focus:{
					border_color:C,
				},
			},
		},
		select:{
			transition_dt:0.1,
			value_animated:0,
			font:UI.Font(UI.font_name,UI.font_size),
			padding:12,spacing:12,
			combo_box_padding:40,
			slider_style:{
				transition_dt:0.1,
				tolerance:2,
				w:32,h_slider:8,
				bgcolor:[{x:0,y:0,color:0xffbbbbbb},{x:0,y:1,color:0xffdddddd}],
				round:8,
				color:C,
				padding:0,
				middle_bar:{
					w:16,h:8,
					round:8,
					color:0xffffffff, border_width:2, border_color:0xff444444,
				},
			},
			button_style:{
				transition_dt:0.1,
				color:0xffffffff,border_color:C,
				round:8,border_width:1.5,padding:12,
				$:{
					out:{
						text_color:C,
					},
					over:{
						text_color:C_dark,
					},
					down:{
						text_color:C_dark,
					},
					////////////////////
					checked_out:{
						color:C,
						text_color:0xffffffff,
					},
					checked_over:{
						color:C,
						text_color:0xffffffff,
					},
					checked_down:{
						color:C,
						text_color:0xffffffff,
					},
				}
			},
		},
		animation_node:{transition_dt:0.1},
	};
};

UI.SetCaret=function(obj,x,y,w,h,C,dt){
	//uses absolute coords
	UI.InsertJSDrawCall(UI.HackCallback(function(){
		if(obj.caret_state>0&&obj.m_window_has_focus){
			UI.DrawCaret(obj.caret_x,obj.caret_y,obj.caret_w,obj.caret_h,obj.caret_C)
		}
		if(obj.m_window_has_focus&&obj.caret_input_rect){
			UI.SDL_SetTextInputRect(
				obj.caret_input_rect[0],obj.caret_input_rect[1],obj.caret_input_rect[2],obj.caret_input_rect[3]);
		}
	}))
	var clip_rect=UI.GetCliprect();
	var x1=x+w;
	var y1=y+h;
	x=Math.max(x,clip_rect.x*UI.pixels_per_unit);
	y=Math.max(y,clip_rect.y*UI.pixels_per_unit);
	x1=Math.min(x1,(clip_rect.x+clip_rect.w)*UI.pixels_per_unit);
	y1=Math.min(y1,(clip_rect.y+clip_rect.h)*UI.pixels_per_unit);
	obj.caret_x=x;
	obj.caret_y=y;
	obj.caret_w=Math.max(x1-x,0);
	obj.caret_h=Math.max(y1-y,0);
	obj.caret_C=C;
	obj.caret_state=2;
	obj.caret_dt=dt;
	obj.caret_is_set=1
	obj.caret_input_rect=[
		(UI.sub_window_offset_x+x)/UI.SDL_bad_coordinate_corrector,
		(UI.sub_window_offset_y+y)/UI.SDL_bad_coordinate_corrector,
		w/UI.SDL_bad_coordinate_corrector,h/UI.SDL_bad_coordinate_corrector];
	UI.SDL_SetTextInputRect(
		obj.caret_input_rect[0],obj.caret_input_rect[1],obj.caret_input_rect[2],obj.caret_input_rect[3]);
};

UI.ChooseScalingFactor=function(obj){
	if(!UI.is_real){
		UI.pixels_per_unit=1
		return;
	}
	if(obj.pixels_per_unit){
		obj.pixels_per_unit=obj.pixels_per_unit;
	}else{
		var display_mode=UI.SDL_GetCurrentDisplayMode();
		var design_screen_dim=obj.designated_screen_size||Math.min(obj.w,obj.h)||1600;
		var screen_dim=Math.min(display_mode.w,display_mode.h);
		if(!(screen_dim>0)){
			//hack for emscripten
			screen_dim=1080;
		}
		//console.log('display_mode:',display_mode.w,display_mode.h);
	}
	UI.pixels_per_unit=screen_dim/design_screen_dim;
	UI.ResetRenderer(UI.pixels_per_unit,obj.gamma||2.2);
	//UI.LoadStaticImages(UI.rc);
	////wipe out initialization routines for security
	//UI.LoadPackedTexture=null;
	//UI.LoadStaticImages=null;
};

UI.ReportSDLResolutionLies=function(lying_by){
	UI.SDL_bad_coordinate_corrector=lying_by
	UI.pixels_per_unit*=lying_by
	UI.ResetRenderer(UI.pixels_per_unit);
	UI.LoadStaticImages(UI.rc);
	return 0;
}

W.Window=function(id,attrs){
	var obj=UI.Keep(id,attrs);
	//the dpi is not per-inch,
	if(!UI.pixels_per_unit){
		UI.ChooseScalingFactor(obj)
	}
	if(!obj.__hwnd){
		//no default event handler for the window
		obj.__hwnd=UI.SDL_CreateWindow(obj.title||"untitled",obj.x||UI.SDL_WINDOWPOS_CENTERED,obj.y||UI.SDL_WINDOWPOS_CENTERED,obj.w*UI.pixels_per_unit,obj.h*UI.pixels_per_unit, obj.flags);
		if(obj.icon){
			UI.SDL_SetWindowIcon(obj.__hwnd,obj.icon);
		}
		UI.m_window_map[obj.__hwnd.toString()]=obj
	}
	//defer the innards painting to the first OnPaint - need the GL context
	UI.context_paint_queue.push(obj);
	UI.BeginPaint(obj.__hwnd,obj);//EndPaint in UI.End()
	UI.FlushGLCalls();
	obj.x=0;
	obj.y=0;
	obj.caret_is_set=0
	if(!UI.is_real){
		UI.sandbox_main_window=obj.__hwnd;
		UI.sandbox_main_window_w=obj.w*UI.pixels_per_unit;
		UI.sandbox_main_window_h=obj.h*UI.pixels_per_unit;
		UI.Clear(obj.bgcolor||0xffffffff);
	}
	UI.sub_window_stack=[[0,0,obj.w_in_pixels,obj.h_in_pixels,UI.pixels_per_unit]];
	return obj;
}

W.FillRect=function(id,attrs){
	UI.StdAnchoring(id,attrs);
	UI.DrawBitmap(0,attrs.x,attrs.y,attrs.w,attrs.h,attrs.color);
	return attrs;
}

W.Bitmap=function(id,attrs){
	UI.StdAnchoring(id,attrs);
	UI.DrawBitmap(UI.rc[attrs.file]||0,attrs.x,attrs.y,attrs.w||0,attrs.h||0,attrs.color||0xffffffff);
	return attrs;
}

W.Text=function(id,attrs){
	if(attrs.w==undefined&&attrs.h==undefined&&attrs.anchor==undefined){
		UI.DrawTextControlImmediate(attrs,attrs.x,attrs.y,attrs.color||0xffffffff)
	}else{
		if(!attrs.__layout){UI.LayoutText(attrs);}
		attrs.w=(attrs.w||attrs.w_text);
		attrs.h=(attrs.h||attrs.h_text);
		UI.StdAnchoring(id,attrs);
		UI.DrawTextControl(attrs,attrs.x,attrs.y,attrs.color||0xffffffff)
	}
	return attrs
};

W.RoundRect=function(id,attrs){
	UI.StdAnchoring(id,attrs);
	UI.RoundRect(attrs)
	return attrs;
}

/*
*auto-sizing* - "layout current object"
Text(,,TR({}))
TR remembers the "last thing" and the next call fetches its w,h
if(attrs.w)
*/

////////////////////////////////////////
//user input
W.Hotkey=function(id,attrs){
	if(!attrs.action){return;}
	if(UI.TranslateHotkey){attrs.key=UI.TranslateHotkey(attrs.key);}
	UI.context_hotkeys.push(attrs);
	return attrs;
}

W.Region=function(id,attrs,proto){
	//attrs is needed to track OnClick and stuff, *even if we don't store any var*
	var obj=UI.Keep(id,attrs,proto);
	UI.StdAnchoring(id,obj);
	return W.PureRegion(id,obj)
}

UI.sub_window_offset_x=0
UI.sub_window_offset_y=0
W.PureRegion=function(id,obj){
	if(obj.OnTextInput||obj.OnKeyDown){
		if(!UI.context_tentative_focus||(UI.context_tentative_focus.default_focus||0)<(obj.default_focus||0)){
			UI.context_tentative_focus=obj;
		}
	}
	if(obj==UI.nd_focus){
		UI.context_focus_is_a_region=1
	}
	if(obj==UI.nd_mouse_over){
		UI.context_mouse_over_is_a_region=1;
	}
	obj.sub_window_offset_x=UI.sub_window_offset_x
	obj.sub_window_offset_y=UI.sub_window_offset_y
	obj.sub_window_scale=UI.pixels_per_unit
	obj.region_clip_rect=UI.GetCliprect()
	UI.context_regions.push(obj);
	obj.region___hwnd=UI.context_window.__hwnd;
	return obj;
}

W.RestoreRegion=function(obj){
	if(obj.OnTextInput||obj.OnKeyDown){
		if(!UI.context_tentative_focus||(UI.context_tentative_focus.default_focus||0)<(obj.default_focus||0)){
			UI.context_tentative_focus=obj;
		}
	}
	if(obj==UI.nd_focus){
		UI.context_focus_is_a_region=1
	}
	if(obj==UI.nd_mouse_over){
		UI.context_mouse_over_is_a_region=1;
	}
	UI.context_regions.push(obj);
}

////////////////////////////////////////
//utility
W.Group=function(id,attrs){
	/*
	item_template
	item_object
	items:
		id
		object_type
		is_hidden
	*/
	var obj=UI.Keep(id,attrs);
	UI.StdAnchoring(id,obj);
	return W.PureGroup(obj)
}

W.PureGroup=function(obj,ending_hint){
	/////////
	var items=obj.items||[];
	var item_template=obj.item_template||{};
	var selection=obj.selection||{};
	var sel_obj_temps;
	var item_template_keys=[];
	for(var key in item_template){
		item_template_keys.push(key);
	}
	//layouting: just set layout_direction and layout_spacing
	UI.Begin(obj);
	obj.layout_auto_anchor=null;
	for(var i=0;i<items.length;i++){
		var items_i=items[i];
		if(!items_i.id){items_i.id="$"+i.toString();}
		if(items_i.is_hidden){continue;}
		var itemobj_i=obj[items_i.id];
		if(itemobj_i&&itemobj_i.is_hidden){
			itemobj_i.__kept=1;
			continue;
		}
		if(!obj[items_i.id]){
			var obj_temp={}
			obj_temp.x=0;obj_temp.y=0;//for layouting
			for(var j=0;j<item_template_keys.length;j++){
				var key=item_template_keys[j];
				obj_temp[key]=item_template[key];
			}
			for(var key in items_i){
				obj_temp[key]=items_i[key];
			}
			obj_temp.selected=selection[obj_temp.id];
			itemobj_i=(obj_temp.object_type)(obj_temp.id,obj_temp);
		}else{
			items_i.x=0;items_i.y=0;
			items_i.selected=selection[items_i.id];
			itemobj_i=(item_template.object_type)(items_i.id,items_i);
		}
		itemobj_i.__kept=1;
	}
	obj.layout_auto_anchor=null;
	UI.End(ending_hint);
	//delete the non-kept
	for(var key in obj){
		//check for leading $
		if(key.charCodeAt(0)==0x24){
			var child=obj[key];
			if(child&&child.__kept){
				child.__kept=0;
			}else{
				delete obj[key];
			}
		}
	}
	return obj;
}

//todo: clipping/scrolling/zooming host with phone awareness

////////////////////////////////////////
//widgets
W.Label=function(id,attrs){
	UI.StdStyling(id,attrs,attrs,"label")
	W.Text(id,attrs)
	return attrs
}

W.Button_prototype={
	value:0,
	use_measured_dims:1,
	icon_text_align:'center',
	icon_text_valign:'center',
	//////////////////
	OnMouseOver:function(){this.mouse_state="over";UI.Refresh();},
	OnMouseOut:function(){this.mouse_state="out";UI.Refresh();},
	OnMouseDown:function(){UI.CaptureMouse(this);this.mouse_state="down";UI.Refresh();},
	OnMouseUp:function(){UI.ReleaseMouse(this);this.mouse_state="over";UI.Refresh();},
	OnClick:function(){
		this.OnChange(this.value?0:1);
	},
	OnChange:function(value){this.value=value;},
};

UI.MeasureIconText=function(obj){
	//size estimation
	var text_dim=UI.MeasureText(obj.font,obj.text)
	var padding=(obj.padding||0);
	return {w:(text_dim.w+padding*2),h:text_dim.h+padding*2};
}

W.DrawIconText=function(id,obj,attrs){
	//size estimation
	var tmp=obj.w;
	var tmph=obj.h
	obj.w=1e17;//for UI.LayoutText
	obj.h=1e17;//for UI.LayoutText
	UI.LayoutText(obj);
	obj.w=tmp;
	obj.h=tmph;
	var padding=(obj.padding||0);
	if(obj.use_measured_dims){
		obj.w=(attrs.w||(obj.w_text+padding*2));
		obj.h=(attrs.h||(obj.h_text+padding*2));
		UI.StdAnchoring(id,obj);
	}
	//////////////////
	//rendering
	UI.RoundRect(obj);
	var alg=(obj.icon_text_align||'center');
	var inner_w=obj.w_text;
	var x;
	if(alg=='left'){
		x=obj.x+padding;
	}else if(alg=='center'){
		x=obj.x+(obj.w-inner_w)*0.5;
	}else{
		x=obj.x+(obj.w-inner_w-padding);
	}
	var inner_h=obj.h_text;
	var y;
	alg=(obj.icon_text_valign||'center');
	if(alg=='up'){
		y=obj.y+padding;
	}else if(alg=='center'){
		y=obj.y+(obj.h-inner_h)*0.5;
	}else{
		y=obj.y+(obj.h-inner_h-padding);
	}
	UI.DrawTextControl(obj,x,y,obj.text_color||0xffffffff)
};

var g_icon_text_from_style=["w_icon","h_icon","padding","icon_text_align","icon_text_valign","icon_color","text_color","x","y","w","h","font"];
var g_icon_text_from_data=["icon","text"];
W.DrawIconTextEx=function(style,data){
	var obj={};
	for(var i=0;i<g_icon_text_from_style.length;i++){
		var id=g_icon_text_from_style[i];
		obj[id]=style[id];
	}
	for(var i=0;i<g_icon_text_from_data.length;i++){
		var id=g_icon_text_from_data[i];
		obj[id]=data[id];
	}
	obj.color=0;
	obj.border_color=0;
	W.DrawIconText("",obj,obj);
};

W.DrawTooltip=function(obj,alpha){
	if(alpha==undefined){alpha=1;}
	UI.TopMostWidget(function(){
		var tstyle=(obj.tooltip_style||UI.default_styles.tooltip)
		var dim=UI.MeasureText(tstyle.font,obj.tooltip)
		dim.w+=tstyle.padding*2
		dim.h+=tstyle.padding*2
		var x,y;
		if(obj.tooltip_placement=='right'){
			x=obj.x+obj.w+tstyle.spacing;
			y=obj.y+(obj.h-dim.h)*0.5;
		}else{
			x=obj.x+(obj.w-dim.w)*0.5;
			if(obj.tooltip_placement=='up'){
				y=obj.y-dim.h-tstyle.spacing
			}else{
				y=obj.y+obj.h+tstyle.spacing
			}
			//x=Math.max(Math.min(x,UI.context_window.w-dim.w),0)
			var subwin=UI.sub_window_stack[UI.sub_window_stack.length-1];
			x=Math.max(Math.min(x,subwin[2]/subwin[4]-dim.w),0);
		}
		UI.RoundRect({
			x:x,y:y,w:dim.w+tstyle.shadow_size,h:dim.h+tstyle.shadow_size,
			color:UI.fade_rgba(tstyle.shadow_color,alpha),
			round:tstyle.round,
			border_width:-tstyle.shadow_size,
		})
		var dim_triangle;
		if(tstyle.triangle_font){
			dim_triangle=UI.MeasureText(tstyle.triangle_font,obj.tooltip_placement=='right'?"\u25C0":"\u25B2")
		}
		if(tstyle.triangle_font2){
			if(obj.tooltip_placement=='right'){
				UI.PushCliprect(x-dim_triangle.w*0.5,obj.y,dim_triangle.w*0.5,obj.h)
				UI.DrawChar(tstyle.triangle_font2,x-dim_triangle.w*0.5,
					obj.y+(obj.h-dim_triangle.h)*0.5,
					UI.fade_rgba(tstyle.border_color,alpha),0x25C0)
				UI.PopCliprect()
			}else{
				UI.PushCliprect(obj.x,y-dim_triangle.h*0.5,obj.w,dim_triangle.h*0.5)
				UI.DrawChar(tstyle.triangle_font2,obj.x+(obj.w-dim_triangle.w)*0.5,
					y-dim_triangle.h*0.5,
					UI.fade_rgba(tstyle.border_color,alpha),0x25B2)
				UI.PopCliprect()
			}
		}
		UI.RoundRect({
			x:x,y:y,w:dim.w,h:dim.h,
			color:UI.fade_rgba(tstyle.color,alpha),
			round:tstyle.round,
			border_color:UI.fade_rgba(tstyle.border_color,alpha),
			border_width:tstyle.border_width,
		})
		if(tstyle.triangle_font){
			if(obj.tooltip_placement=='right'){
				UI.DrawChar(tstyle.triangle_font,x-dim_triangle.w*0.5,
					obj.y+(obj.h-dim_triangle.h)*0.5,
					UI.fade_rgba(tstyle.color,alpha),0x25C0)
			}else{
				UI.DrawChar(tstyle.triangle_font,obj.x+(obj.w-dim_triangle.w)*0.5,
					y-dim_triangle.h*0.5,
					UI.fade_rgba(tstyle.color,alpha),0x25B2)
			}
		}
		W.Text("",{x:x+tstyle.padding,y:y+tstyle.padding,font:tstyle.font,text:obj.tooltip,color:UI.fade_rgba(tstyle.text_color,alpha),flags:14})
	})
}

W.Button=function(id,attrs){
	//////////////////
	//styling
	var obj=UI.Keep(id,attrs,W.Button_prototype);
	UI.StdStyling(id,obj,attrs, "button",(obj.value?"checked_":"")+(obj.mouse_state||"out"));
	W.DrawIconText(id,obj,attrs)
	if(obj.tooltip){
		var tooltip_alpha=0
		if(((obj.mouse_state||'out')!='out'||obj.show_tooltip_override)){
			tooltip_alpha=1
		}
		UI.Begin(obj)
		var anim=W.AnimationNode("tooltip_animation",{transition_dt:obj.transition_dt,
			tooltip_alpha:tooltip_alpha})
		UI.End(obj)
		if(anim.tooltip_alpha>0){
			W.DrawTooltip(obj,anim.tooltip_alpha)
		}
	}
	return W.PureRegion(id,obj);
}

UI.HL_DISPLAY_MODE_RECT=0
UI.HL_DISPLAY_MODE_RECTEX=1
UI.HL_DISPLAY_MODE_EMBOLDEN=2
UI.HL_DISPLAY_MODE_TILDE=3
var g_regexp_newline=new RegExp("\n","g")
var g_regexp_dos2unix=new RegExp("\r\n","g")
W.Edit_prototype={
	plugin_class:'widget',
	//////////
	scale:1,
	mouse_wheel_speed:4,
	scroll_x:0,
	scroll_y:0,
	x_updown:0,
	//////////
	mouse_cursor:"ibeam",
	default_style_name:"edit",
	//color:0xff000000,
	//bgcolor_selection:0xffffe0d0,
	caret_is_wrapped:0,
	GetHandlerID:function(name){
		return this.ed.m_handler_registration[name];
	},
	GetCharacterHeightAtCaret:function(){
		var ccnt1=this.sel1.ccnt;
		if(this.caret_is_wrapped>0){
			ccnt1=this.ed.SnapToCharBoundary(ccnt1+1,1)
		}
		return this.ed.GetCharacterHeightAt(ccnt1);
	},
	AutoScroll:function(mode){
		//'show' 'center' 'center_if_hidden'
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		var ed=this.ed;
		var ed_caret=this.GetCaretXY();
		var y_original=this.scroll_y;
		var ccnt_tot=ed.GetTextSize();
		var ytot=ed.XYFromCcnt(ccnt_tot).y+ed.GetCharacterHeightAt(ccnt_tot);
		var wc=UI.GetCharacterAdvance(ed.GetDefaultFont(),32);
		var hc=this.GetCharacterHeightAtCaret();
		var page_height=this.h;
		var h_top_hint=(this.h_top_hint||0);
		if(mode=='bound'){
			var x0=this.scroll_x;
			var y0=this.scroll_y;
			this.scroll_x=Math.max(this.scroll_x,0);
			this.scroll_y=Math.max(Math.min(this.scroll_y,ytot-page_height),0);
			if(this.scroll_x!=x0||this.scroll_y!=y0){
				this.scrolling_animation=undefined;
			}
			return;
		}
		if(mode!='show'){
			//print(ed_caret,this.sel1.ccnt,this.ed.GetTextSize())
			if(this.scroll_y>ed_caret.y||this.scroll_y<=ed_caret.y-page_height||mode=='center'){
				//mode 'center_if_hidden': only center when the thing was invisible
				this.scroll_y=ed_caret.y-(page_height-hc)/2;
			}
		}
		this.scroll_y=Math.min(this.scroll_y,ed_caret.y-h_top_hint)
		if(ed_caret.y-this.scroll_y>=page_height-hc){
			this.scroll_y=(ed_caret.y-(page_height-hc));
		}
		var wwidth=this.w-wc*(this.right_side_autoscroll_margin||1);
		if(mode!='show'&&this.sel0.ccnt!=this.sel1.ccnt){
			//make sure sel0 shows up
			this.scroll_x=Math.min(this.scroll_x,Math.min(ed.XYFromCcnt(ccnt0).x,ed_caret.x))
		}
		this.scroll_x=Math.min(this.scroll_x,ed_caret.x);
		if(this.scroll_x>0&&ed_caret.x<wwidth){
			//aggressive x scroll on short lines
			var ccnt_lend=this.SeekXY(1e17,ed_caret.y);
			if(ed.XYFromCcnt(ccnt_lend).x<=wwidth){
				this.scroll_x=0;
			}
		}
		if(ed_caret.x-this.scroll_x>=wwidth){
			//going over the right
			var chneib=ed.GetUtf8CharNeighborhood(ccnt1);
			var ch=chneib[1];
			if(ch==13||ch==10){
				//line end, don't do anything funny
				this.scroll_x=ed_caret.x-wwidth;
			}else{
				//line middle, scroll to a more middle-ish position
				var right_side_autoscroll_space=(this.right_side_autoscroll_space||16);
				this.scroll_x=ed_caret.x-wwidth+Math.min(wwidth/2,right_side_autoscroll_space*wc);
			}
		}
		this.scroll_x=Math.max(this.scroll_x,0);
		this.scroll_y=Math.max(Math.min(this.scroll_y,ytot-page_height),0);
		this.x_updown=ed_caret.x
		if(this.disable_scrolling_x){this.scroll_x=0;}
		if(this.disable_scrolling_y){this.scroll_y=0;}
		//TestTrigger(KEYCODE_ANY_MOVE)
	},
	OnMouseWheel:function(event){
		var ed=this.ed;
		var ccnt_tot=ed.GetTextSize();
		var ytot=ed.XYFromCcnt(ccnt_tot).y+ed.GetCharacterHeightAt(ccnt_tot);
		var hc=this.GetCharacterHeightAtCaret();
		var page_height=this.h;
		this.scroll_y=Math.max(Math.min(this.scroll_y-hc*event.y*this.mouse_wheel_speed,ytot-page_height),0);
		UI.Refresh()
	},
	SkipInvisibles:function(ccnt,side){
		return this.ed.MoveToBoundary(ccnt,side,"invisible_boundary")
	},
	SnapToValidLocation:function(ccnt,side){
		//var ed=this.ed;
		//var ccnt_cb=ed.SnapToCharBoundary(ccnt,side);
		////return ccnt_cb;
		//var xy=ed.XYFromCcnt(ccnt_cb);
		//var ccnt_vb=ed.SeekXY(xy.x,xy.y);
		//return ccnt_vb;
		return this.ed.MoveToBoundary(this.ed.SnapToCharBoundary(ccnt,side),-1,"invisible_boundary")
	},
	////////////////////////////
	SeekLC:function(line,column){
		var ed=this.ed;
		return ed.Bisect(ed.m_handler_registration["line_column"],[line,column],"ll");
	},
	SeekLineBelowKnownPosition:function(ccnt_known,line_known,line_new){
		var ed=this.ed;
		return ed.Bisect(ed.m_handler_registration["line_column"],[line_new,0, line_known,ccnt_known],"llll");
	},
	GetLC:function(ccnt){
		var ed=this.ed;
		return ed.GetStateAt(ed.m_handler_registration["line_column"],ccnt,"ll");
	},
	GetEnhancedHome:function(ccnt){
		var ccnt_lhome=this.SeekLC(this.GetLC(ccnt)[0],0);
		//in case there is \r, we need to snap before it, thus the -1
		return this.SnapToValidLocation(this.ed.MoveToBoundary(ccnt_lhome,1,"space"),-1);
	},
	GetEnhancedEnd:function(ccnt){
		var ccnt_lend=this.SeekLC(this.GetLC(ccnt)[0],1e17);
		if(ccnt_lend>ccnt&&this.ed.GetText(ccnt_lend-1,1)=="\n"){ccnt_lend--;}
		return this.SnapToValidLocation(this.ed.MoveToBoundary(ccnt_lend,-1,"space"),-1)
	},
	////////////////////////////
	GetCaretXY:function(){
		var ed=this.ed;
		var xy0=ed.XYFromCcnt(this.sel1.ccnt)
		if(this.caret_is_wrapped>0){
			//chicken-egg: no this.GetCharacterHeightAtCaret() here
			xy0.x=0
			xy0.y+=ed.GetCharacterHeightAt(this.sel1.ccnt);
		}else if(this.caret_is_wrapped<0){
			//the *other* mode
			xy0.y-=ed.GetCharacterHeightAt(this.sel1.ccnt);
			xy0.x=this.displayed_wrap_width
		}
		//print(this.caret_is_wrapped,xy0.x,xy0.y)
		return xy0;
	},
	GetIMECaretXY:function(){
		var ed=this.ed;
		var xy0=ed.XYFromCcnt(this.sel1.ccnt)
		xy0.x+=ed.m_caret_offset
		if(this.caret_is_wrapped>0){
			//chicken-egg: no this.GetCharacterHeightAtCaret() here
			xy0.x=0
			xy0.y+=ed.GetCharacterHeightAt(this.sel1.ccnt);
		}else if(this.caret_is_wrapped<0){
			//the *other* mode
			xy0.y-=ed.GetCharacterHeightAt(this.sel1.ccnt);
			xy0.x=this.displayed_wrap_width
		}
		return xy0;
	},
	SeekXY:function(x,y){
		var ed=this.ed
		var ccnt=ed.SeekXY(x,y)
		//skip invisible
		var ccnt_maybe_newline=this.SkipInvisibles(ccnt,-1)
		if(ccnt_maybe_newline>0&&ed.GetUtf8CharNeighborhood(ccnt_maybe_newline)[0]==10){
			//it's a newline before
			var xy=ed.XYFromCcnt(ccnt)
			if(xy.y>y){
				//we should have landed on the previous line
				ccnt=ccnt_maybe_newline-1
			}
		}
		if(ccnt==ed.GetTextSize()){
			//eof special case for mousing
			ccnt=ed.SeekXY(x,ed.XYFromCcnt(ccnt).y)
		}
		return ccnt
	},
	MoveCursorToXY:function(x,y){
		var ed=this.ed;
		this.sel1.ccnt=this.SeekXY(x,y);
		this.caret_is_wrapped=0;
		//print("IsAtLineWrap",this.sel1.ccnt,ed.IsAtLineWrap(this.sel1.ccnt),ed.GetUtf8CharNeighborhood(this.sel1.ccnt))
		var lwmode=ed.IsAtLineWrap(this.sel1.ccnt)
		if(lwmode){
			var x0=this.GetCaretXY().x;
			this.caret_is_wrapped=lwmode;
			var x1=this.GetCaretXY().x;
			this.caret_is_wrapped=(Math.abs(x1-x)<Math.abs(x0-x)?lwmode:0);
		}
	},
	OnChange:function(){},
	OnSelectionChange:function(){},
	/////////////////
	CallHooks:function(name){
		var hk=this.m_event_hooks[name];
		if(hk){
			for(var i=hk.length-1;i>=0;i--){
				hk[i].call(this)
			}
		}
	},
	CallOnChange:function(){
		this.caret_is_wrapped=0;
		this.AutoScroll('show')
		this.CallHooks('change')
		this.OnChange(this);
		this.m_user_just_typed_char=0
	},
	CallOnSelectionChange:function(){
		this.CallHooks('selectionChange')
		this.OnSelectionChange(this);
		this.m_user_just_typed_char=0
	},
	UserTypedChar:function(){
		if(!this.m_user_just_typed_char){
			this.m_user_just_typed_char=1;
			this.CallHooks('userTypeChar');
		}
	},
	/////////////////
	AddEventHandler:function(s_key,faction){
		UI.assert(!this.m_transient_hotkeys);
		if(this.m_event_hooks[s_key]){
			this.m_event_hooks[s_key].push(faction)
		}else{
			this.m_additional_hotkeys.push({'key':s_key,'action':faction})
			//UI.assert(this.m_additional_hotkeys.length<1000);
		}
	},
	AddTransientHotkey:function(s_key,faction){
		this.m_transient_hotkeys.push({'key':s_key,'action':UI.HackCallback(faction)})
	},
	/////////////////
	Copy:function(){
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		var ed=this.ed
		if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
		if(ccnt0<ccnt1){
			UI.SDL_SetClipboardText(ed.GetText(ccnt0,ccnt1-ccnt0))
		}
	},
	Cut:function(){
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		var ed=this.ed
		if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
		if(ccnt0<ccnt1){
			this.Copy();//UI.SDL_SetClipboardText(ed.GetText(ccnt0,ccnt1-ccnt0))
			this.HookedEdit([ccnt0,ccnt1-ccnt0,null])
			this.CallOnChange();
			UI.Refresh();
			return 1;
		}
		return 0;
	},
	Paste:function(){
		var stext=UI.SDL_GetClipboardText()
		if(UI.Platform.ARCH=="win32"||UI.Platform.ARCH=="win64"){
			stext=stext.replace(g_regexp_dos2unix,"\n");
		}
		if(this.is_single_line){
			stext=stext.replace(g_regexp_newline," ");
		}
		this.OnTextInput({"text":stext,"is_paste":1})
	},
	Undo:function(){
		var ed=this.ed;
		var ret=ed.Undo()
		if(ret&&ret.sz>=0){
			this.sel0.ccnt=ret.ccnt;
			this.sel1.ccnt=ret.ccnt+ret.sz;
			this.AutoScroll("center_if_hidden");
		}
		this.CallOnChange();
		UI.Refresh();
	},
	Redo:function(){
		var ed=this.ed;
		var ret=ed.Undo("redo")
		if(ret&&ret.sz>=0){
			this.sel0.ccnt=ret.ccnt;
			this.sel1.ccnt=ret.ccnt+ret.sz;
			this.AutoScroll("center_if_hidden");
		}
		this.CallOnChange();
		UI.Refresh();
	},
	////////////////////////////
	HookedEdit:function(ops){if(this.read_only){return;};this.ed.Edit(ops);},
	GetSelection:function(){
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
		return [ccnt0,ccnt1];
	},
	CreateTransientHighlight:function(attrs){
		var ed=this.ed
		var p0=ed.CreateLocator(0,-1);p0.undo_tracked=0;
		var p1=ed.CreateLocator(0,-1);p1.undo_tracked=0;
		var hl=ed.CreateHighlight(p0,p1);
		for(var key in attrs){
			hl[key]=attrs[key]
		}
		return [p0,p1,hl]
	},
	AddAdditionalPlugins:function(){},
	Init:function(){
		var ed=this.ed;
		if(!ed){
			//don't allow plugins to extend state_handlers for now
			this.m_additional_hotkeys=(this.additional_hotkeys||[])
			if(!this.m_event_hooks){this.m_event_hooks={}}
			//userTypeChar differs from text input in that 'paste' doesn't count
			this.m_event_hooks['userTypeChar']=[]
			this.m_event_hooks['selectionChange']=[]
			this.m_event_hooks['change']=[]
			this.m_event_hooks['editorCreate']=[]
			this.m_event_hooks['afterRender']=[]
			this.m_event_hooks['doubleClick']=[]
			this.m_event_hooks['tripleClick']=[]
			var plugins=this.plugins;
			if(plugins){
				for(var i=0;i<plugins.length;i++){
					plugins[i].call(this)
				}
			}
			this.AddAdditionalPlugins()
			ed=UI.CreateEditor(this);
			if(this.text){ed.Edit([0,0,this.text],1);}
			this.sel0=ed.CreateLocator(0,-1);this.sel0.undo_tracked=1;
			this.sel1=ed.CreateLocator(0,-1);this.sel1.undo_tracked=1;
			this.ed=ed;
			this.sel_hl=ed.CreateHighlight(this.sel0,this.sel1);
			this.sel_hl.color=this.bgcolor_selection;
			this.sel_hl.invertible=1;
			ed.m_caret_locator=this.sel1;
			this.CallHooks('editorCreate')
		}
	},
	OnTextEdit:function(event){
		if(event.text.length){
			this.ed.m_IME_overlay=event;
		}else{
			this.ed.m_IME_overlay=undefined;
		}
		UI.Refresh()
	},
	ProcessHotkeysInput:function(hk,event){
		var sel0_ccnt=this.sel0.ccnt;
		var sel1_ccnt=this.sel1.ccnt;
		if(!hk){return 0;}
		for(var i=hk.length-1;i>=0;i--){
			var hki=hk[i]
			if(event.text==hki.key){
				if(!hki.action.call(this,hki.key)){
					//0 for cancel
					//if(sel0_ccnt!=this.sel0.ccnt||sel1_ccnt!=this.sel1.ccnt){
					//	this.CallOnSelectionChange()
					//}
					UI.Refresh()
					return 1
				}
			}
		}
		return 0
	},
	OnTextInput:function(event){
		if(this.read_only){return;}
		var ed=this.ed;
		var ccnt0=this.sel0.ccnt;var sel0_ccnt=ccnt0
		var ccnt1=this.sel1.ccnt;var sel1_ccnt=ccnt1
		if(ccnt0>ccnt1){var tmp=ccnt1;ccnt1=ccnt0;ccnt0=tmp;}
		var ops=[ccnt0,ccnt1-ccnt0,event.text];
		if(event.text.length==1&&!event.is_paste){
			//ASCII key events
			if(this.ProcessHotkeysInput(this.m_transient_hotkeys,event)){return;}
			if(this.ProcessHotkeysInput(this.m_additional_hotkeys,event)){return;}
		}
		if(event.text){
			this.HookedEdit(ops)
		}
		//for hooked case, need to recompute those
		var lg=Duktape.__byte_length(ops[2]);
		this.sel0.ccnt=ops[0]+lg;
		this.sel1.ccnt=ops[0]+lg;
		this.AutoScroll("show");
		this.CallOnChange();
		this.UserTypedChar();
		this.ed.m_IME_overlay=undefined;
		UI.Refresh()
	},
	ProcessHotkeysKeyDown:function(hk,event){
		var sel0_ccnt=this.sel0.ccnt;
		var sel1_ccnt=this.sel1.ccnt;
		var IsHotkey=UI.IsHotkey
		if(!hk){return 0;}
		for(var i=hk.length-1;i>=0;i--){
			var hki=hk[i]
			if(hki.key.length!=1&&IsHotkey(event,hki.key)){
				if(!hki.action.call(this,hki.key,event)){
					//return 0 for "cancel default"
					if(sel0_ccnt!=this.sel0.ccnt||sel1_ccnt!=this.sel1.ccnt){
						this.CallOnSelectionChange()
					}
					UI.Refresh()
					return 1;
				}
			}
		}
		return 0
	},
	OnKeyDown:function(event){
		//allow multiple keys
		/*
		mouse messages
		*/
		var ed=this.ed;
		var IsHotkey=UI.IsHotkey;
		var is_shift=event.keymod&(UI.KMOD_LSHIFT|UI.KMOD_RSHIFT);
		var sel0=this.sel0;
		var sel1=this.sel1;
		var this_outer=this;
		var sel0_ccnt=sel0.ccnt;
		var sel1_ccnt=sel1.ccnt;
		var epilog=UI.HackCallback(function(){
			if(!is_shift){sel0.ccnt=sel1.ccnt;}
			this_outer.AutoScroll("show");
			UI.Refresh();
		});
		//if(UI.enable_timing){UI.TimingEvent("m_transient_hotkeys "+this.m_transient_hotkeys.length);}
		if(this.ProcessHotkeysKeyDown(this.m_transient_hotkeys,event)){return;}
		//if(UI.enable_timing){UI.TimingEvent("m_additional_hotkeys "+this.m_additional_hotkeys.length);}
		if(this.ProcessHotkeysKeyDown(this.m_additional_hotkeys,event)){return;}
		//if(UI.enable_timing){UI.TimingEvent("the rest of OnKeyDown");}
		if(0){
		}else if(IsHotkey(event,"UP SHIFT+UP")){
			var ed_caret=this.GetCaretXY();
			var bk=this.x_updown;
			this.MoveCursorToXY(this.x_updown,ed_caret.y-1.0);
			epilog();
			this.x_updown=bk;
		}else if(IsHotkey(event,"DOWN SHIFT+DOWN")){
			var hc=this.GetCharacterHeightAtCaret();
			var ed_caret=this.GetCaretXY();
			var bk=this.x_updown;
			this.MoveCursorToXY(this.x_updown,ed_caret.y+hc);
			epilog();
			this.x_updown=bk;
		}else if(IsHotkey(event,"LEFT SHIFT+LEFT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,-1);
			var lwmode=ed.IsAtLineWrap(ccnt);
			if(this.caret_is_wrapped>0){
				this.caret_is_wrapped=0;
				epilog();
			}else if(this.caret_is_wrapped>lwmode){
				this.caret_is_wrapped=lwmode;
				epilog();
			}else{
				if(this.same_line_only_left_right){
					if(this.ed.GetUtf8CharNeighborhood(ccnt)[0]==10){
						return;
					}
				}
				if(ccnt>0){
					sel1.ccnt=this.SnapToValidLocation(ccnt-1,-1);
					epilog();
				}
				lwmode=ed.IsAtLineWrap(sel1.ccnt);
				this.caret_is_wrapped=Math.max(lwmode,0);
			}
		}else if(IsHotkey(event,"RIGHT SHIFT+RIGHT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,1);
			var lwmode=ed.IsAtLineWrap(ccnt);
			if(this.caret_is_wrapped<0){
				this.caret_is_wrapped=0;
				epilog();
			}else if(this.caret_is_wrapped<lwmode){
				this.caret_is_wrapped=lwmode;
				epilog();
			}else{
				if(ccnt<ed.GetTextSize()){
					sel1.ccnt=this.SnapToValidLocation(ccnt+1,1);
					if(this.same_line_only_left_right){
						if(this.ed.GetUtf8CharNeighborhood(sel1.ccnt)[0]==10){
							sel1.ccnt=ccnt;
							return;
						}
					}
					epilog();
				}
				lwmode=ed.IsAtLineWrap(sel1.ccnt);
				this.caret_is_wrapped=Math.min(lwmode,0);
			}
		}else if(IsHotkey(event,"CTRL+LEFT SHIFT+CTRL+LEFT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,-1);
			if(ccnt>0){
				sel1.ccnt=this.SnapToValidLocation(ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt-1,-1),-1,this.precise_ctrl_lr_stop?"precise_ctrl_lr_stop":"ctrl_lr_stop"),-1)
				this.caret_is_wrapped=ed.IsAtLineWrap(sel1.ccnt);
				epilog();
			}
		}else if(IsHotkey(event,"CTRL+RIGHT SHIFT+CTRL+RIGHT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,1);
			if(ccnt<ed.GetTextSize()){
				sel1.ccnt=this.SnapToValidLocation(ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt+1,1),1,this.precise_ctrl_lr_stop?"precise_ctrl_lr_stop":"ctrl_lr_stop"),1)
				this.caret_is_wrapped=0;
				epilog();
			}
		}else if(IsHotkey(event,"BACKSPACE")||IsHotkey(event,"DELETE")){
			var ccnt0=sel0.ccnt;
			var ccnt1=sel1.ccnt;
			var is_backspace=(IsHotkey(event,"BACKSPACE"));
			var bk_m_user_just_typed_char=this.m_user_just_typed_char
			if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
			if(ccnt0==ccnt1){
				if(IsHotkey(event,"BACKSPACE")){
					if(ccnt0>0){ccnt0=ed.SnapToCharBoundary(this.SkipInvisibles(ccnt0,-1)-1,-1);}
				}else{
					if(ccnt1<ed.GetTextSize()){ccnt1=ed.SnapToCharBoundary(this.SkipInvisibles(ccnt1,1)+1,1);}
				}
			}
			if(ccnt0<ccnt1){
				this.HookedEdit([ccnt0,ccnt1-ccnt0,null])
				this.CallOnChange()
				if(is_backspace){
					//this.m_user_just_typed_char=bk_m_user_just_typed_char
					if(bk_m_user_just_typed_char){
						this.UserTypedChar();
					}
				}
				UI.Refresh();
				return;
			}
		}else if(IsHotkey(event,"CTRL+HOME SHIFT+CTRL+HOME")){
			sel1.ccnt=0;
			this.caret_is_wrapped=0;
			epilog()
		}else if(IsHotkey(event,"CTRL+END SHIFT+CTRL+END")){
			sel1.ccnt=ed.GetTextSize();
			this.caret_is_wrapped=0;
			epilog()
		}else if(IsHotkey(event,"CTRL+A")){
			sel0.ccnt=0;
			sel1.ccnt=ed.GetTextSize();
			this.caret_is_wrapped=0;
			UI.Refresh();
		}else if(IsHotkey(event,"RETURN RETURN2")){
			if(this.is_single_line){
				this.OnEnter.call(this)
			}else{
				this.OnTextInput({"text":"\n"})
			}
		}else if(IsHotkey(event,"TAB")&&this.tab_is_char&&sel0.ccnt==sel1.ccnt){
			this.OnTextInput({"text":"\t"})
		}else if(IsHotkey(event,"HOME SHIFT+HOME")){
			var ed_caret=this.GetCaretXY();
			var ccnt_lhome=this.SeekXY(0,ed_caret.y);
			var ccnt_rehome=this.GetEnhancedHome(ccnt_lhome)//sel1_ccnt
			var ccnt_ehome=Math.max(ccnt_rehome,ccnt_lhome);
			if(sel1.ccnt==ccnt_ehome&&ccnt_lhome==ccnt_ehome){
				sel1.ccnt=ccnt_rehome;
				this.caret_is_wrapped=0;
			}else if(sel1.ccnt==ccnt_ehome||ccnt_lhome==ccnt_ehome){
				sel1.ccnt=ccnt_lhome;
				this.caret_is_wrapped=Math.max(ed.IsAtLineWrap(this.sel1.ccnt),0);
			}else{
				sel1.ccnt=ccnt_ehome;
				this.caret_is_wrapped=0;
			}
			epilog();
		}else if(IsHotkey(event,"END SHIFT+END")){
			var ed_caret=this.GetCaretXY();
			var ccnt_lend=this.SeekXY(1e17,ed_caret.y);
			var ccnt_reend=this.GetEnhancedEnd(ccnt_lend)//sel1_ccnt
			var ccnt_eend=Math.min(ccnt_reend,ccnt_lend);
			if(sel1.ccnt==ccnt_eend&&ccnt_lend==ccnt_eend){
				sel1.ccnt=ccnt_reend;
				this.caret_is_wrapped=0;
			}else if(sel1.ccnt==ccnt_eend||ccnt_lend==ccnt_eend){
				sel1.ccnt=ccnt_lend;
				this.caret_is_wrapped=Math.min(ed.IsAtLineWrap(this.sel1.ccnt),0);
			}else{
				sel1.ccnt=ccnt_eend;
				this.caret_is_wrapped=0;
			}
			epilog();
		}else if(IsHotkey(event,"PGUP SHIFT+PGUP")){
			var hc=this.GetCharacterHeightAtCaret();
			var bk=this.x_updown;
			var ed_caret=this.GetCaretXY();
			this.MoveCursorToXY(this.x_updown,ed_caret.y-Math.max(Math.floor(this.h/hc-(this.page_guard_lines||0)),1)*hc);
			var ed_caret2=this.GetCaretXY();
			this.scroll_y+=ed_caret2.y-ed_caret.y
			epilog();
			this.x_updown=bk;
		}else if(IsHotkey(event,"PGDN SHIFT+PGDN")){
			var hc=this.GetCharacterHeightAtCaret();
			var bk=this.x_updown;
			var ed_caret=this.GetCaretXY();
			this.MoveCursorToXY(this.x_updown,ed_caret.y+Math.max(Math.floor(this.h/hc-(this.page_guard_lines||0)),1)*hc);
			var ed_caret2=this.GetCaretXY();
			this.scroll_y+=ed_caret2.y-ed_caret.y
			epilog();
			this.x_updown=bk;
		}else if(IsHotkey(event,"CTRL+C")||IsHotkey(event,"CTRL+INSERT")){
			this.Copy()
		}else if(IsHotkey(event,"CTRL+X")||IsHotkey(event,"SHIFT+DELETE")){
			if(this.Cut()){return;}
		}else if(IsHotkey(event,"CTRL+V")||IsHotkey(event,"SHIFT+INSERT")){
			this.Paste()
			return;
		}else if(IsHotkey(event,"CTRL+Z")||IsHotkey(event,"ALT+BACKSPACE")){
			this.Undo();
		}else if(IsHotkey(event,"SHIFT+CTRL+Z")||IsHotkey(event,"CTRL+Y")){
			this.Redo();
		}else{
		}
		if(sel0_ccnt!=sel0.ccnt||sel1_ccnt!=sel1.ccnt){
			this.CallOnSelectionChange()
		}
	},
	SetSelection:function(ccnt0,ccnt1){
		this.sel0.ccnt=ccnt0
		this.sel1.ccnt=ccnt1
		//this.AutoScroll(mode&&this.sel0.ccnt==ccnt0&&this.sel1.ccnt==ccnt1?"center":"center_if_hidden");
		this.AutoScroll("center_if_hidden");
		this.caret_is_wrapped=0
		UI.Refresh()
		//this.CallOnSelectionChange()
	},
	SetCaretTo:function(ccnt){
		this.SetSelection(ccnt,ccnt)
	},
	////////////////////////////
	OnMouseDown:function(event){
		var x0=event.x-this.x+this.scroll_x
		var y0=event.y-this.y+this.scroll_y
		var ccnt_clicked=this.SeekXY(x0,y0);
		this.dragging_shift=0;
		if(event.clicks==2){
			//double-click
			this.sel0.ccnt=this.SnapToValidLocation(this.ed.MoveToBoundary(this.ed.SnapToCharBoundary(Math.max(this.SkipInvisibles(ccnt_clicked,-1),0),-1),-1,"word_boundary_left"),-1)
			this.sel1.ccnt=this.SnapToValidLocation(this.ed.MoveToBoundary(this.ed.SnapToCharBoundary(Math.min(this.SkipInvisibles(ccnt_clicked,1),this.ed.GetTextSize()),1),1,"word_boundary_right"),1)
			this.caret_is_wrapped=Math.min(this.ed.IsAtLineWrap(this.sel1.ccnt),0);
			this.CallOnSelectionChange()
			//UI.Refresh()
			//return
		}else if(event.clicks>=3){
			//triple-click
			var line=this.GetLC(ccnt_clicked)[0]
			this.sel0.ccnt=this.SeekLC(line,0)
			this.sel1.ccnt=this.SeekLC(line+1,0)
			this.caret_is_wrapped=Math.min(this.ed.IsAtLineWrap(this.sel1.ccnt),0);
			this.CallOnSelectionChange()
			//UI.Refresh()
			//return
		}else{
			if(event.button==UI.SDL_BUTTON_RIGHT&&this.OnRightClick){
				//prevent sel when right clicking selected text
				var sel=this.GetSelection();
				if(ccnt_clicked>=sel[0]&&ccnt_clicked<=sel[1]){
					UI.SetFocus(this)
					UI.Refresh()
					return;
				}
			}
			if(!UI.IsPressed("LSHIFT")&&!UI.IsPressed("RSHIFT")){
				this.sel0.ccnt=ccnt_clicked;
			}else{
				this.dragging_shift=1;
			}
			this.sel1.ccnt=ccnt_clicked;
			this.caret_is_wrapped=Math.min(this.ed.IsAtLineWrap(this.sel1.ccnt),0);
			this.CallOnSelectionChange()
		}
		this.is_dragging=1
		this.dragging_ccnt0=this.sel0.ccnt;
		this.dragging_ccnt1=this.sel1.ccnt;
		UI.SetFocus(this)
		UI.CaptureMouse(this)
		if(event.clicks>=3){
			this.CallHooks('tripleClick')
		}else if(event.clicks==2){
			this.CallHooks('doubleClick')
		}
		UI.Refresh()
	},
	OnMouseMove:function(event){
		if(!this.is_dragging){return;}
		var x1=event.x-this.x+this.scroll_x
		var y1=event.y-this.y+this.scroll_y
		var ccnt=this.SeekXY(x1,y1);
		if(!this.dragging_shift&&ccnt>=this.dragging_ccnt0&&ccnt<=this.dragging_ccnt1){
			this.sel0.ccnt=this.dragging_ccnt0;
			this.sel1.ccnt=this.dragging_ccnt1;
		}else{
			if(!this.dragging_shift){
				if(ccnt<this.dragging_ccnt0){
					this.sel0.ccnt=this.dragging_ccnt1;
				}else{
					this.sel0.ccnt=this.dragging_ccnt0;
				}
			}
			this.sel1.ccnt=ccnt;
		}
		this.AutoScroll('show')
		this.CallOnSelectionChange()
		UI.Refresh()
	},
	OnMouseUp:function(event){
		if(!this.is_dragging){return;}
		UI.ReleaseMouse(this)
		this.is_dragging=0
		UI.Refresh()
	},
	RenderAsWidget:function(id,x,y,w,h){
		this.x=x;
		this.y=y;
		this.w=w;
		this.h=h;
		W.PureRegion(id,this)
		if(this.show_background){
			UI.DrawBitmap(0,this.x,this.y,this.w,this.h,this.bgcolor);
		}
		if(!this.ed){this.Init();}
		this.m_transient_hotkeys=[]
		this.sel_hl.color=this.bgcolor_selection;
		var scale=this.scale*UI.pixels_per_unit;
		var scroll_x=this.scroll_x;
		var scroll_y=this.scroll_y;
		var ed=this.ed;
		if(this.scroll_transition_dt>0){
			UI.Begin(this)
				var anim=W.AnimationNode("scrolling_animation",{transition_dt:this.scroll_transition_dt,
					scroll_x:scroll_x,
					scroll_y:scroll_y})
			UI.End()
			scroll_x=anim.scroll_x
			scroll_y=anim.scroll_y
		}
		this.visible_scroll_x=scroll_x
		this.visible_scroll_y=scroll_y
		//Render takes absolute coords
		//var bkcolor
		//if(this.read_only){
		//	bkcolor=this.sel_hl.color
		//	this.sel_hl.color=0
		//}
		ed.Render({x:scroll_x,y:scroll_y,w:this.w/this.scale,h:this.h/this.scale, scr_x:this.x*UI.pixels_per_unit,scr_y:this.y*UI.pixels_per_unit, scale:scale, obj:this});
		this.CallHooks('afterRender')
		//if(this.read_only){
		//	this.sel_hl.color=bkcolor
		//	return this
		//}else{
		if(UI.HasFocus(this)){
			var ed_caret=this.GetIMECaretXY();
			var x_caret=this.x*UI.pixels_per_unit+(ed_caret.x-scroll_x)*scale;
			var y_caret=this.y*UI.pixels_per_unit+(ed_caret.y-scroll_y)*scale;
			UI.PushCliprect(this.x,this.y,this.w,this.h)
			UI.SetCaret(UI.context_window,
				x_caret,y_caret,
				this.caret_width*scale,this.GetCharacterHeightAtCaret()*scale,
				this.caret_color,this.caret_flicker);
			UI.PopCliprect()
		}
	},
};
W.Edit=function(id,attrs,proto){
	var obj=UI.Keep(id,attrs,proto||W.Edit_prototype);
	UI.StdStyling(id,obj,attrs, obj.default_style_name,UI.HasFocus(obj)?"focus":"blur");
	UI.StdAnchoring(id,obj);
	obj.RenderAsWidget(id,obj.x,obj.y,obj.w,obj.h)
	return obj;
	//}
};

/////////////////////////////////////////////////////
//menu
W.MenuItem_prototype={
	use_measured_dims:1,
	icon_text_align:'left',
	icon_text_valign:'center',
	OnMouseOver:function(){
		this.parent.selection={};
		this.parent.selection[this.id]=1;
		UI.Refresh();
	},
	OnClick:function(){
		this.action();
		this.parent.Close()
	},
	action:function(){}
};
W.MenuItem=function(id,attrs){
	var obj=UI.Keep(id,attrs,W.MenuItem_prototype);
	obj.parent=UI.context_parent;
	//styling should draw the box
	UI.StdStyling(id,obj,attrs, "menu_item",obj.parent.selection[obj.id]?"over":"out");
	W.DrawIconText(id,obj,attrs)
	return W.PureRegion(id,obj);
}

W.Menu_prototype={
	item_template:{object_type:W.MenuItem},
	layout_direction:"down",
	//////////////
	IDFromSelection:function(){
		var items=this.items;
		var sel=this.selection;
		///////
		var items_clean=[];
		var sel_id=0;
		for(var i=0;i<items.length;i++){
			var item_i=items[i];
			if(sel[item_i.id]){
				sel_id=items_clean.length;
			}
			if(item_i.is_hidden){continue;}
			items_clean.push(item_i);
		}
		if(sel_id>=items_clean.length){sel_id--;}
		return {'sel_id':sel_id,'items_clean':items_clean};
	},
	//////////////
	OnKeyDown:function(event){
		var sdesc=this.IDFromSelection();
		var sel_id=sdesc.sel_id;
		var items_clean=sdesc.items_clean;
		if(0){
		}else if(UI.IsHotkey(event,"UP")){
			sel_id=(sel_id+items_clean.length-1)%Math.max(items_clean.length,1)
			this.selection={};
			this.selection[items_clean[sel_id].id]=1;
			UI.Refresh()
		}else if(UI.IsHotkey(event,"DOWN")){
			sel_id=(sel_id+1)%Math.max(items_clean.length,1)
			this.selection={};
			this.selection[items_clean[sel_id].id]=1;
			UI.Refresh()
		}else if(UI.IsHotkey(event,"RETURN")){
			this[items_clean[sel_id].id].action();
			if(UI.HasFocus(this)){
				this.Close();
			}
			UI.Refresh()
		}else{
			//todo: left/right for submenu
		}
	},
	//OnBlur:function(obj){
	//	if(!obj.in_on_blur){
	//		obj.in_on_blur=1;
	//		UI.SetFocus(this.nd_focus_saved)
	//		obj.in_on_blur=0;
	//	}
	//},
	Popup:function(){
		var sv=UI.nd_focus;
		while(sv&&sv.nd_focus_saved){
			sv=sv.nd_focus_saved
		}
		this.nd_focus_saved=sv;
		UI.SetFocus(this)
		UI.Refresh()
	},
	Close:function(){
		UI.SetFocus(this.nd_focus_saved)
		UI.Refresh()
	},
};
W.Menu=function(id,attrs){
	var obj=UI.Keep(id,attrs,W.Menu_prototype);
	UI.StdStyling(id,obj,attrs, "menu",UI.HasFocus(obj)?"focus":"blur");
	UI.StdAnchoring(id,obj);
	if(UI.HasFocus(obj)){UI.TopMostWidget(function(){
		//auto width/height, drag-scrolling
		if(!obj.selection){
			obj.selection={"$0":1};
		}
		UI.RoundRect(obj);
		W.PureRegion(id,obj)//region goes before children
		W.PureGroup(obj)
		if(!attrs.w||!attrs.h){
			//auto sizing
			var w_max=0;
			var h_tot=0;
			var spacing=(obj.layout_spacing||0);
			var items=obj.items
			for(var i=0;i<items.length;i++){
				var item_i=obj[items[i].id];
				if(!item_i||item_i.is_hidden){continue;}
				//todo: left part vs right part
				var dim_i=UI.MeasureIconText(item_i)
				w_max=Math.max(w_max,dim_i.w)
				h_tot+=dim_i.h+spacing;
			}
			var obj_w=(attrs.w||w_max+(obj.w_base||0));
			var obj_h=(attrs.h||Math.min(h_tot,obj.h_max||h_tot));
			if(obj.w!=obj_w){obj.w=obj_w;UI.Refresh()}
			if(obj.h!=obj_h){obj.h=obj_h;UI.Refresh()}
		}
		return obj;
	})}else{
		return obj;
	}
}

//a sensible default style - qpad
W.ComboBox_prototype={
	value:undefined,
	OnMouseOver:function(){this.mouse_state="over";UI.Refresh();},
	OnMouseOut:function(){this.mouse_state="out";UI.Refresh();},
	OnClick:function(){
		if(UI.HasFocus(this.menu)){
			this.menu.Close()
		}else{
			this.menu.selection={};
			for(var i=0;i<this.items.length;i++){
				if(this.items[i].text==this.value){
					this.menu.selection["$"+i]=1
					break
				}
			}
			this.menu.Popup();
		}
	},
	GetSubStyle:function(){
		return this.edit&&UI.HasFocus(this.edit)?"focus":"blur"
	},
	OnChange:function(value){this.value=value;},
};
W.ComboBox=function(id,attrs){
	//items same as menu, need explicit w h
	var obj=UI.StdWidget(id,attrs,"combobox",W.ComboBox_prototype)
	if(obj.value==undefined){
		if(obj.items.length){
			obj.value=obj.items[0].text;
		}else{
			obj.value="";
		}
	}
	//show active
	UI.RoundRect(obj);
	W.PureRegion(id,obj);
	//it's a styling problem, just do it manually, ignore the generality
	W.DrawIconTextEx(obj,{x:obj.x,y:obj.y,w:obj.w,h:obj.h, font:obj.font,text:obj.value,color:obj.text_color})
	UI.Begin(obj)
		W.Text("-",{anchor:UI.context_parent,anchor_align:"right",anchor_valign:"center",font:obj.label_font,x:obj.padding,text:"▼",color:obj.icon_color})
		W.Menu("menu",{
			'x':obj.x, 'y':obj.y+obj.h, 'w':obj.w,
			'items':obj.items,
			'style':obj.menu_style,
			'item_template':{object_type:W.MenuItem,action:function(){
				if(obj.OnChange){obj.OnChange.call(obj,this.text)};
				obj.menu.Close();
			}},
		})
		if(obj.has_edit){
			//todo: edit vs menu
		}
	UI.End()
	return obj;
}

W.Slider_prototype={
	value:0,
	OnMouseDown:function(event){
		this.mouse_state="down";
		this.BeginContinuousChange()
		UI.CaptureMouse(this)
		this.OnMouseMove(event)
		UI.Refresh();
	},
	OnMouseUp:function(){
		UI.ReleaseMouse(this)
		this.EndContinuousChange()
		this.mouse_state="over";
		UI.Refresh();
	},
	OnMouseOver:function(){
		this.mouse_state="over";
		UI.Refresh();
	},
	OnMouseOut:function(){
		this.mouse_state="out";
		UI.Refresh();
	},
	OnMouseMove:function(event){
		if(this.mouse_state!="down"){return;}
		this.OnChange(Math.max(Math.min((event.x-this.x)/this.w,1),0),1)
		UI.Refresh()
	},
	OnChange:function(value){this.value=value;},
	BeginContinuousChange:function(){},
	EndContinuousChange:function(){},
	GetSubStyle:function(){
		return this.mouse_state||"out"
	},
};
W.SliderLabel_prototype={
	OnMouseDown:function(event){
		var obj=this.parent
		obj.BeginContinuousChange()
		UI.CaptureMouse(this)
		this.is_dragging=1
		this.anchor_x=event.x
		this.anchor_w_value=obj.w*obj.value
	},
	OnMouseUp:function(){
		this.is_dragging=0
		UI.ReleaseMouse(this)
		this.parent.EndContinuousChange()
	},
	OnMouseMove:function(event){
		if(!this.is_dragging){return;}
		var obj=this.parent
		obj.OnChange(Math.max(Math.min((event.x-this.anchor_x+this.anchor_w_value)/obj.w,1),0),1)
		UI.Refresh()
	},
}
W.Slider=function(id,attrs){
	var obj=UI.StdWidget(id,attrs,"slider",W.Slider_prototype)
	//how do we set the initial value reliably?
	//we don't: we provide an OnChange callback, and put the value itself on the fcall - just like boxdocs
	var w_value=obj.value*obj.w;
	var h_slider=obj.h_slider||obj.h;
	W.PureRegion(id,obj)
	if(obj.bgcolor||obj.border_color){
		UI.RoundRect({
			x:obj.x, y:obj.y+(obj.h-h_slider)*0.5, w:obj.w, h:h_slider, round:obj.round,
			color:obj.bgcolor, border_width:obj.border_width, border_color:obj.border_color})
	}
	if(obj.color){
		UI.PushCliprect(obj.x,obj.y,w_value,obj.h)
		UI.RoundRect({
			x:obj.x+obj.padding, y:obj.y+(obj.h-h_slider)*0.5+obj.padding, w:obj.w-obj.padding*2, h:h_slider-obj.padding*2, round:obj.round,
			color:obj.color})
		UI.PopCliprect()
	}
	if(obj.middle_bar){
		UI.RoundRect({
			x:obj.x+w_value-obj.middle_bar.w*0.5, y:obj.y+(obj.h-h_slider-obj.middle_bar.h)*0.5, w:obj.middle_bar.w, h:h_slider+obj.middle_bar.h, round:obj.middle_bar.round,
			color:obj.middle_bar.color, border_width:obj.middle_bar.border_width, border_color:obj.middle_bar.border_color})
	}
	if(obj.label_text){
		//var tmp={w:1e17,h:1e17,font:obj.label_font,text:obj.label_text}
		//UI.LayoutText(tmp);
		var dim=UI.MeasureText(obj.label_font,obj.label_text.toString());
		UI.Begin(obj)
			var obj_label={}
			obj_label.x=obj.x+w_value-dim.w*0.5;
			obj_label.y=obj.y+obj.h-dim.h*(obj.label_raise||0)
			obj_label.w=dim.w
			obj_label.h=dim.h
			obj_label.parent=obj
			W.Region("label",obj_label,W.SliderLabel_prototype)
			//UI.DrawTextControl(tmp,obj_label.x,obj_label.y,obj.label_color)
			W.Text("",{x:obj_label.x,y:obj_label.y,font:obj.label_font,text:obj.label_text,color:obj.label_color})
		UI.End()
	}
	return obj;
}

W.EditBox_prototype={
	value:"",
	focus_state:"blur",
	OnClick:function(){
		if(!this.OnChange){return;}
		if(this.focus_state=="blur"){
			this.bak_value=this.value;
			this.focus_state="focus"
		}else{
			this.focus_state="blur";
		}
		UI.Refresh()
	},
	//OnFocus:function(){this.focus_state="focus";},
	//OnBlur:function(){this.focus_state="blur";},
	OnChange:function(value){this.value=value;},
	GetSubStyle:function(){
		return this.focus_state;
	}
}
W.EditBox=function(id,attrs){
	//coulddo: tab-stop system
	var obj=UI.StdWidget(id,attrs,"edit_box",W.EditBox_prototype)
	UI.RoundRect(obj)
	W.PureRegion(id,obj)
	var dim_text=UI.MeasureIconText({font:obj.font,text:obj.value||obj.hint_text||"0"})
	UI.Begin(obj)
		if(!obj.OnChange){obj.focus_state="blur";}
		if(obj.focus_state=="focus"||UI.nd_focus==obj){
			var is_newly_created=!obj.edit;
			W.Edit("edit",{
				anchor:'parent',anchor_align:"left",anchor_valign:"center",
				x:obj.padding,y:0,w:obj.w-obj.padding*2,h:dim_text.h,
				font:obj.font, color:obj.text_color, text:obj.value,
				hint_text:obj.hint_text,
				is_single_line:1,
				obj:obj,
				additional_hotkeys:[{key:"ESCAPE",action:function(){
					//cancel the change
					var obj=this.obj
					obj.OnChange(obj.bak_value)
					obj.bak_value=undefined
					obj.focus_state="blur"
					UI.Refresh()
				}}],
				OnBlur:function(){
					if(obj.focus_state=="blur"){return;}
					obj.focus_state="blur"
					obj.OnChange(obj.edit.ed.GetText())
					UI.Refresh()
				},
				OnEnter:function(){
					this.OnBlur()
				},
			});
			obj.focus_state="focus";
			if(is_newly_created){
				UI.SetFocus(obj.edit)
				obj.edit.sel0.ccnt=0
				obj.edit.sel1.ccnt=obj.edit.ed.GetTextSize()
				UI.InvalidateCurrentFrame()
			}
			if(obj.tab_stop){
				UI.TabStop(obj.edit);
			}
		}else{
			//text
			obj.edit=undefined
			W.Text("text",{
				anchor:'parent',anchor_align:"left",anchor_valign:"center",
				x:obj.padding,y:0,w:obj.w-obj.padding*2,h:dim_text.h,
				font:obj.font, color:obj.value?obj.text_color:obj.hint_color, text:obj.value||obj.hint_text,
				});
			if(obj.tab_stop){
				UI.TabStop(obj);
			}
		}
	UI.End()
	return obj;
}

W.Select_prototype={
	value:0,
	OnChange:function(value){this.value=value;},
	OnClick:function(){
		this.OnChange(this.value?0:1)
		UI.Refresh()
	},
}
W.Select=function(id,attrs){
	//the value is the numerical id
	var obj0=UI.GetPreviousState(id);
	if(obj0){attrs.value_animated=obj0.value;}
	var obj=UI.StdWidget(id,attrs,"select",W.Select_prototype)
	UI.Begin(obj)
		var items=obj.items;
		if(items.length==2&&items[0]==0&&items[1]==1){
			//on/off slider for the [0,1] case
			W.Slider("impl",{
				anchor:'parent',anchor_align:'right',anchor_valign:'fill',x:obj.slider_style.middle_bar.w*0.25,y:0,
				style:obj.slider_style,
				value:obj.value_animated,
				OnChange:function(value){obj.OnChange(value<0.5?0:1)},
				OnMouseDown:function(event){
					this.clickdet_value=obj.value;
					this.clickdet_x=event.x
					this.clickdet_y=event.y
					this.clickdet_out=0
					W.Slider_prototype.OnMouseDown.call(this,event)
				},
				OnMouseMove:function(event){
					var dx=this.clickdet_x-event.x
					var dy=this.clickdet_y-event.y
					if(dx*dx+dy*dy>=this.tolerance*this.tolerance){
						this.clickdet_out=1
					}
					W.Slider_prototype.OnMouseMove.call(this,event)
				},
				OnMouseUp:function(event){
					W.Slider_prototype.OnMouseUp.call(this,event)
					if(this.clickdet_value==obj.value&&!this.clickdet_out){
						obj.OnChange(obj.value?0:1)
					}
				},
			})
		}else{
			var w_needed=(items.length-1)*obj.spacing+obj.padding*2;
			var w_max=0
			for(var i=0;i<items.length;i++){
				var w_i=UI.MeasureIconText({font:obj.font,text:items[i].toString()}).w;
				w_needed+=w_i
				w_max=Math.max(w_max,w_i)
			}
			//note: right-align the control
			if(w_needed<=obj.w){
				//multi-button
				//they are actual buttons, just clipped buttons, which handles animation, mouseover and stuff
				//the main widget only draws the separator lines and the outmost border
				//we don't really need separator lines yet, can't AA them anyway
				var x_base=obj.x+obj.w-w_needed+obj.button_style.border_width,x_cur=x_base;
				for(var i=0;i<items.length;i++){
					var x_last=x_cur
					if(i==0){
						x_cur+=obj.padding;
					}else{
						x_cur+=obj.spacing*0.5;
					}
					var x_text=x_cur;
					x_cur+=UI.MeasureIconText({font:obj.font,text:items[i].toString()}).w
					if(i<items.length-1){
						x_cur+=obj.spacing*0.5;
					}else{
						x_cur+=obj.padding;
					}
					UI.PushCliprect(x_last,obj.y,x_cur-x_last,obj.h)
					var btn_i=W.Button("$"+i,{
						x:x_base+obj.button_style.border_width,y:obj.y+obj.button_style.border_width,w:w_needed-obj.button_style.border_width*2,h:obj.h-obj.button_style.border_width*2,
						text:"",style:obj.button_style,
						value:(obj.value==i),i:i,
						OnChange:function(value){
							obj.OnChange(this.i);
							UI.Refresh()
						},
					})
					btn_i.x=x_last
					btn_i.w=x_cur-x_last
					W.Text("",{
						anchor:'parent',anchor_align:'left',anchor_valign:'center',
						x:x_text-obj.x,y:0,
						font:obj.font,text:items[i],
						color:btn_i.text_color,
					})
					UI.PopCliprect()
				}
			}else{
				//combobox - loop-search for numerical value
				var items2=[];
				w_max+=obj.combo_box_padding
				for(var i=0;i<items.length;i++){
					items2[i]={'text':items[i]};
				}
				W.ComboBox("combobox",{
					x:obj.x+obj.w-w_max,y:obj.y,w:w_max,h:obj.h,
					items:items2,value:items[obj.value],
					style:obj.combo_box_style,
					OnChange:function(value){
						for(var i=0;i<items.length;i++){
							if(items[i]==value){
								obj.OnChange(i);
								break
							}
						}
					},
				});
			}
		}
	UI.End()
	return obj;
}

//use anchor_placement to determine the side, default to right
W.AutoHidePanel_prototype={
	anchor_placement:'right',
	layout_direction:'inside',layout_align:'left',layout_valign:'up',
	position:undefined,velocity:0,max_velocity:1000,acceleration:1500,oob_scale:0.5,oob_limit:20,dt_threshold:0.1,velocity_to_target_threshold:0.3,
	Simulate:function(){
		var a=this.dragging_samples;
		var size=(this.anchor_placement=='left'||this.anchor_placement=='right'?this.w:this.h)
		var tick_last=this.sim_tick;
		this.sim_tick=Duktape.__ui_get_tick();
		if(tick_last==undefined){tick_last=this.sim_tick-1/UI.animation_framerate;}
		var sim_dt=Duktape.__ui_seconds_between_ticks(tick_last,this.sim_tick)
		if(a){
			var n=a.length,p0=Math.max(n-3,0);
			this.position=a[n-1].x-a[0].x+this.anchored_position
			if(this.position<0){this.position*=this.oob_scale;}
			if(this.position>=size){this.position=(this.position-size)*this.oob_scale+size;}
			this.position=Math.max(Math.min(this.position,size+this.oob_limit),-this.oob_limit)
			var p2=Math.max(n-2,0)
			var dt=a[n-1].t-a[p2].t
			if(p2>0&&dt<=0){
				p2--
				dt=a[n-1].t-a[p2].t
			}
			if(dt<=0){
				this.velocity=0;
			}else{
				this.velocity=(a[n-1].x-a[Math.max(n-2,0)].x)/dt;
			}
			if(this.position<size*0.5){
				this.target_position=0
			}else{
				this.target_position=size
			}
		}else{
			if(this.target_position!=undefined){
				this.position+=this.velocity*sim_dt;
				if(this.position<-this.oob_limit){this.velocity=0;this.position=-this.oob_limit}
				if(this.position>size+this.oob_limit){this.velocity=0;this.position=size+this.oob_limit}
				if(this.target_position>this.position){
					if(this.velocity<0){this.velocity=0}
					this.velocity+=this.acceleration*sim_dt;
				}else{
					if(this.velocity>0){this.velocity=0}
					this.velocity-=this.acceleration*sim_dt;
				}
				if(Math.abs(this.target_position-this.position)<this.velocity*2*sim_dt){
					this.velocity=0
					this.position=this.target_position
					this.target_position=undefined;
				}
				UI.AutoRefresh()
			}
		}
		this.velocity=Math.max(Math.min(this.velocity,this.max_velocity),-this.max_velocity)
	}
}
W.AutoHidePanel_knob_prototype={
	OnMouseDown:function(event){
		var obj=this.owner
		if(!obj.dragging_samples){
			obj.dragging_samples=[];
		}
		//anchor like normal widgets
		obj.anchored_position=obj.position
		obj.tick0=Duktape.__ui_get_tick()
		this.OnMouseMove(event);
		obj.target_position=(obj.position==0?obj.w:0)
		UI.CaptureMouse(this)
	},
	OnMouseMove:function(event){
		var obj=this.owner;
		if(!obj.dragging_samples){return;}
		var t=Duktape.__ui_seconds_between_ticks(obj.tick0,Duktape.__ui_get_tick());
		var side=(obj.anchor_placement=='left'||obj.anchor_placement=='right'?"x":"y")
		if(obj.anchor_placement=='left'||obj.anchor_placement=='up'){
			obj.dragging_samples.push({x:event[side],t:t})
		}else{
			obj.dragging_samples.push({x:-event[side],t:t})
		}
		obj.Simulate()
		UI.Refresh()
	},
	OnMouseUp:function(event){
		UI.ReleaseMouse(this)
		var obj=this.owner;
		var t=Duktape.__ui_seconds_between_ticks(obj.tick0,Duktape.__ui_get_tick());
		if(obj.dragging_samples&&obj.dragging_samples.length){
			var dt=t-obj.dragging_samples[obj.dragging_samples.length-1].t
			obj.velocity*=Math.max(obj.dt_threshold-dt,0)/obj.dt_threshold
		}
		if(obj.velocity>obj.max_velocity*obj.velocity_to_target_threshold){
			var size=(obj.anchor_placement=='left'||obj.anchor_placement=='right'?obj.w:obj.h)
			obj.target_position=size
		}
		if(obj.velocity<-obj.max_velocity*obj.velocity_to_target_threshold){
			obj.target_position=0
		}
		obj.dragging_samples=undefined
		UI.Refresh()
	}
}
var g_inverse_dir={'left':'right','right':'left','up':'down','down':'up','center':'center','fill':'fill'}
W.AutoHidePanel=function(id,attrs){
	var obj=UI.Keep(id,attrs,W.AutoHidePanel_prototype)
	UI.StdStyling(id,obj,attrs,"auto_hide_panel")
	if(obj.position==undefined){obj.position=(obj.initial_position||0);}
	if(obj.anchor_placement=='left'||obj.anchor_placement=='right'){
		obj.x=-obj.position;obj.y=0
		obj.anchor_align=obj.anchor_placement
		obj.anchor_valign='fill'
	}else{
		obj.y=-obj.position;obj.x=0
		obj.anchor_valign=obj.anchor_placement
		obj.anchor_align='fill'
	}
	obj.anchor='parent'
	UI.StdAnchoring(id,obj)
	var is_x=(obj.anchor_placement=='left'||obj.anchor_placement=='right')
	var size=(is_x?obj.w:obj.h)
	//simply place the child object inside Begin / End
	UI.Begin(obj)
		//one of w/h will be overwritten with fill anyway
		if(!obj.dragging_samples){obj.Simulate()}
		W.Region("knob",{
			anchor:'parent',anchor_placement:g_inverse_dir[obj.anchor_placement],anchor_align:obj.anchor_align,anchor_valign:obj.anchor_valign,
			x:is_x?-size:0,y:is_x?0:-size,w:obj.knob_size+size,h:obj.knob_size+size,
			owner:obj
		},W.AutoHidePanel_knob_prototype)
	UI.End("temp")
	return obj;
}

W.ScrollBar_prototype={
	mouse_state:'out',
	OnMouseOver:function(){this.mouse_state="over";UI.Refresh();},
	OnMouseOut:function(){this.mouse_state="out";UI.Refresh();},
	GetSubStyle:function(){
		if(this.bar&&this.bar.mouse_state=="over"){return "over";}
		return this.mouse_state
	},
	///////////////
	value:0,
	OnChange:function(value){this.value=value;},
	OnClick:function(event){
		if(this.bar){
			var y_bar=this.bar.y+this.bar.h*0.5;
			this.OnChange(Math.min(Math.max(this.value+
				(event.y<y_bar?-1:1)*this.page_size/Math.max(this.total_size-this.page_size,1e-15),0),1))
		}
	},
}
W.ScrollBarThingy_prototype={
	dimension:'y',
	mouse_state:'out',
	OnMouseOver:function(){this.mouse_state="over";UI.Refresh();},
	OnMouseOut:function(){this.mouse_state="out";UI.Refresh();},
	OnMouseDown:function(event){
		var owner=this.owner
		this.anchored_value=owner.value
		this.anchored_xy=event[owner.dimension]
		UI.CaptureMouse(this)
	},
	OnMouseUp:function(event){
		UI.ReleaseMouse(this)
		this.anchored_value=undefined
		UI.Refresh()
	},
	OnMouseMove:function(event){
		if(this.anchored_value==undefined){return;}
		var owner=this.owner
		owner.OnChange(Math.min(Math.max(this.anchored_value+(event[owner.dimension]-this.anchored_xy)/this.factor,0),1))
	},
}
W.ScrollBar=function(id,attrs){
	var obj=UI.StdWidget(id,attrs,"scroll_bar",W.ScrollBar_prototype)
	W.PureRegion(id,obj)
	UI.Begin(obj)
		var szbar;
		if(obj.page_size&&obj.total_size){
			szbar=Math.min(obj.page_size/obj.total_size,0.9)
		}else{
			szbar=0.2;
		}
		szbar*=obj[obj.dimension=="y"?"h":"w"]
		szbar=Math.max(szbar,obj.szbar_min)
		if(obj.bgcolor||obj.border_color){
			UI.RoundRect({
				x:obj.x, y:obj.y, w:obj.w, h:obj.h, round:obj.round,
				color:obj.bgcolor, border_width:obj.border_width, border_color:obj.border_color})
		}
		if(obj.middle_bar){
			var rect;
			if(obj.dimension=="y"){
				rect={
					x:obj.x+(obj.w-obj.middle_bar.w)*0.5,
					y:obj.y+(obj.h-szbar)*obj.value, 
					w:obj.middle_bar.w, h:szbar,
					factor:obj.h-szbar,
					round:obj.middle_bar.round,
					color:obj.icon_color||obj.middle_bar.color,
					border_width:obj.middle_bar.border_width, 
					border_color:obj.middle_bar.border_color}
			}else{
				rect={
					x:obj.x+(obj.w-szbar)*obj.value,
					y:obj.y+(obj.h-obj.middle_bar.h)*0.5,
					w:szbar, h:obj.middle_bar.w,
					factor:obj.w-szbar,
					round:obj.middle_bar.round,
					color:obj.icon_color||obj.middle_bar.color, 
					border_width:obj.middle_bar.border_width, 
					border_color:obj.middle_bar.border_color}
			}
			UI.RoundRect(rect)
			W.Region("bar",{x:rect.x,y:rect.y,w:rect.w,h:rect.h,factor:rect.factor,owner:obj},W.ScrollBarThingy_prototype)
			if(obj.OnMouseWheel){
				obj.bar.OnMouseWheel=obj.OnMouseWheel;
			}
		}
	UI.End()
	return obj;
}

//this does not cover multi-sel
W.ListView_prototype={
	position:0,velocity:0,min_velocity:10,max_velocity:1000,acceleration:1500,oob_scale:0.5,oob_limit:20,dt_threshold:0.1,
	damping_rate:0.1,
	dimension:'y',
	//just a velocity
	Simulate:function(){
		var a=this.dragging_samples;
		var size=Math.max(this.dim_tot-(this.dimension=='y'?this.h:this.w),0)
		var tick_last=this.sim_tick;
		this.sim_tick=Duktape.__ui_get_tick();
		if(tick_last==undefined){tick_last=this.sim_tick-1/UI.animation_framerate;}
		var sim_dt=Duktape.__ui_seconds_between_ticks(tick_last,this.sim_tick)
		if(a){
			var n=a.length,p0=Math.max(n-3,0);
			this.position=a[n-1].x-a[0].x+this.anchored_position
			if(this.position<0){this.position*=this.oob_scale;}
			if(this.position>=size){this.position=(this.position-size)*this.oob_scale+size;}
			this.position=Math.max(Math.min(this.position,size+this.oob_limit),-this.oob_limit)
			var p2=Math.max(n-2,0)
			var dt=a[n-1].t-a[p2].t
			if(p2>0&&dt<=0){
				p2--
				dt=a[n-1].t-a[p2].t
			}
			if(dt<=0){
				this.velocity=0;
			}else{
				this.velocity=(a[n-1].x-a[Math.max(n-2,0)].x)/dt;
			}
		}else{
			//if(this.m_autoscroll_goal!=undefined){
			//	if(!this.velocity){this.velocity=0;}
			//	if(this.position<this.m_autoscroll_goal){
			//		this.velocity=this.max_velocity;
			//	}else{
			//		this.velocity=-this.max_velocity;
			//	}
			//	var delta_snap=Math.abs(this.velocity*sim_dt)
			//	if(Math.abs(this.position-this.m_autoscroll_goal)<delta_snap){
			//		this.position=this.m_autoscroll_goal
			//		this.velocity=0
			//		this.m_autoscroll_goal=undefined
			//		UI.AutoRefresh()
			//	}
			//}
			if(this.velocity){
				var old_pos=this.position;
				this.position+=this.velocity*sim_dt;
				if(this.position<-this.oob_limit){this.velocity=0;this.position=-this.oob_limit}
				if(this.position>size+this.oob_limit){this.velocity=0;this.position=size+this.oob_limit}
				if(this.position<0){
					if(this.velocity<0){this.velocity=0}
					this.velocity+=this.acceleration*sim_dt;
				}else if(this.position>size){
					if(this.velocity>0){this.velocity=0}
					this.velocity-=this.acceleration*sim_dt;
				}else{
					this.velocity*=Math.exp(-this.damping_rate*sim_dt)
					if(Math.abs(this.velocity)<this.min_velocity){
						this.velocity=0
					}
				}
				if(old_pos<0&&this.position>=0){
					this.position=0;
					this.velocity=0;
				}else if(old_pos>size&&this.position<=size){
					this.position=size;
					this.velocity=0;
				}
				UI.AutoRefresh()
			}
		}
		this.velocity=Math.max(Math.min(this.velocity,this.max_velocity),-this.max_velocity)
	},
	AutoScroll:function(){
		var sel=this.value
		var dim=this.dimension
		var wh_dim=(dim=='y'?'h':'w')
		var item_sel=this["$"+sel]
		if(item_sel){
			var pos_sel=item_sel[dim]-this[dim]+this.position
			var pos_goal=Math.max(Math.min(this.position,pos_sel),pos_sel+item_sel[wh_dim]-this[wh_dim])
			pos_goal=Math.max(Math.min(pos_goal,this.dim_tot-this[wh_dim]),0)
			//if(pos_goal!=this.position){
			//	this.m_autoscroll_goal=pos_goal
			//}else{
			//	this.m_autoscroll_goal=undefined
			//}
			this.position=(pos_goal||0)
			UI.Refresh()
		}
	},
	OnMouseDown:function(event){
		if(!UI.IS_MOBILE){return;}
		var obj=this
		if(!obj.dragging_samples){
			obj.dragging_samples=[];
		}
		//anchor like normal widgets
		obj.anchored_position=obj.position
		obj.tick0=Duktape.__ui_get_tick()
		this.OnMouseMove(event);
		obj.target_position=(obj.position==0?obj.w:0)
		UI.CaptureMouse(this)
	},
	OnMouseMove:function(event){
		var obj=this;
		if(!obj.dragging_samples){return;}
		var t=Duktape.__ui_seconds_between_ticks(obj.tick0,Duktape.__ui_get_tick());
		obj.dragging_samples.push({x:-event[obj.dimension],t:t})
		obj.Simulate()
		UI.Refresh()
	},
	OnMouseUp:function(event){
		if(!UI.IS_MOBILE){return;}
		UI.ReleaseMouse(this)
		var obj=this;
		var t=Duktape.__ui_seconds_between_ticks(obj.tick0,Duktape.__ui_get_tick());
		if(obj.dragging_samples&&obj.dragging_samples.length){
			var dt=t-obj.dragging_samples[obj.dragging_samples.length-1].t
			obj.velocity*=Math.max(obj.dt_threshold-dt,0)/obj.dt_threshold
		}
		if(obj.velocity>obj.max_velocity*obj.velocity_to_target_threshold){
			var size=(obj.anchor_placement=='left'||obj.anchor_placement=='right'?obj.w:obj.h)
			obj.target_position=size
		}
		if(obj.velocity<-obj.max_velocity*obj.velocity_to_target_threshold){
			obj.target_position=0
		}
		obj.dragging_samples=undefined
		UI.Refresh()
	},
	OnKeyDown:function(event){
		var n=this.items.length
		var value=this.value
		if(UI.IsHotkey(event,this.dimension=="y"?"UP":"LEFT")){
			var p_goal=value;
			for(var p=value-1;p>=0;p--){
				var item_p=this.items[p];
				if(item_p.is_hidden||item_p.no_selection){continue;}
				p_goal=p
				break;
			}
			if(value!=p_goal){this.OnChange(p_goal);}
		}else if(UI.IsHotkey(event,this.dimension=="y"?"DOWN":"RIGHT")){
			var p_goal=value;
			for(var p=value+1;p<n;p++){
				var item_p=this.items[p];
				if(item_p.is_hidden||item_p.no_selection){continue;}
				p_goal=p
				break;
			}
			if(value!=p_goal){this.OnChange(p_goal);}
		}else if(UI.IsHotkey(event,"PGUP")&&this.items.length){
			var obj_sel=this["$"+value]
			var dim=this.dimension
			var wh_dim=(dim=='y'?'h':'w')
			var delta=this[wh_dim]
			var p_goal=value
			for(var p=value-1;p>=0;p--){
				var item_p=this.items[p];
				if(item_p.is_hidden||item_p.no_selection){continue;}
				item_p=this["$"+p];
				if(!item_p){
					p_goal=p
					break;
				}
				p_goal=p
				delta-=item_p[wh_dim]
				if(delta<=0){
					break;
				}
			}
			if(p_goal!=value){this.OnChange(p_goal)}
		}else if(UI.IsHotkey(event,"PGDN")&&this.items.length){
			var obj_sel=this["$"+value]
			var dim=this.dimension
			var wh_dim=(dim=='y'?'h':'w')
			var delta=this[wh_dim]
			var p_goal=value
			for(var p=value;p<this.items.length;p++){
				var item_p=this.items[p];
				if(item_p.is_hidden||item_p.no_selection){continue;}
				item_p=this["$"+p];
				if(!item_p){
					p_goal=p
					break;
				}
				p_goal=p;
				delta-=item_p[wh_dim]
				if(delta<=0){
					break;
				}
			}
			if(p_goal!=value){this.OnChange(p_goal)}
		}else if(UI.IsHotkey(event,"RETURN RETURN2")){
			var obj_sel=this["$"+value]
			if(obj_sel&&obj_sel.OnDblClick){
				obj_sel.OnDblClick();
			}
		}
	},
	///////////////
	value:0,
	mouse_wheel_speed:80,
	OnChange:function(value){
		this.value=value;
		this.AutoScroll()
		UI.Refresh();
	},
	OnMouseWheel:function(event){
		var dim=this.dimension
		var wh_dim=(dim=='y'?'h':'w')
		this.position=Math.max(Math.min(this.position-event.y*this.mouse_wheel_speed,this.dim_tot-this[wh_dim]),0)
		UI.Refresh()
	},
};

W.ListView=function(id,attrs){
	var obj=UI.StdWidget(id,attrs,"list_view",W.ListView_prototype)
	var items=obj.items
	var item_template=(obj.item_template||{});
	var id_changed=0
	if(obj.OnDemand){
		if(obj.real_items){items=obj.real_items}
		var y0=-obj.position;
		var items_new=[]
		var wh_dim=(obj.dimension=='y'?'h':'w')
		var any_changed=0;
		var value_new=obj.value;
		for(var i=0;i<items.length;i++){
			//if(y0>obj[wh_dim]){
			//	for(var j=i;j<items.length;j++){
			//		items_new.push(items[i])
			//	}
			//	break
			//}
			var item_i=items[i]
			if(!item_i){continue;}
			var expanded=obj.OnDemand.call(item_i)
			if(i==obj.value){value_new=items_new.length;}
			if((typeof(expanded))=="string"){
				if(expanded=="drop"){
					//do nothing
					delete obj[item_i.id]
					id_changed=1
					any_changed=1;
				}else{
					UI.assert(expanded=="keep","you must return 'drop' or 'keep' or an array of offsprings")
					if(id_changed){
						delete obj[item_i.id]
						delete item_i.id;
					}
					items_new.push(item_i)
					y0+=(item_i[wh_dim]||item_template[wh_dim]||0)+(obj.layout_spacing||0)
				}
			}else{
				delete obj[item_i.id];
				if(expanded.length!=1){id_changed=1;}
				for(var j=0;j<expanded.length;j++){
					items_new.push(expanded[j])
					y0+=(expanded[j][wh_dim]||item_template[wh_dim]||0)+(obj.layout_spacing||0)
				}
				any_changed=1;
			}
		}
		items=items_new
		obj.items=items
		obj.real_items=items
		if(id_changed&&value_new<items.length){
			obj.value=value_new;
		}
		if(obj.OnDemandSort&&any_changed){
			for(var i=0;i<items.length;i++){
				var item_i=items[i];
				if(item_i){delete obj[item_i.id];}
				item_i.id=undefined;
			}
			obj.OnDemandSort(obj);
			id_changed=1;
		}
	}
	while(obj.value<items.length&&items[obj.value].no_selection){
		obj.value++;
		id_changed=1;
	}
	var dim_tot=obj.layout_spacing*(items.length+1)
	if(obj.dimension=="y"){
		for(var i=0;i<items.length;i++){
			dim_tot+=items[i].h;
			if(items[i].is_hidden){
				dim_tot-=obj.layout_spacing;
			}
		}
		dim_tot=Math.max(dim_tot,0)
		obj.layout_direction="down"
		obj.layout_scroll_x=0
		obj.layout_scroll_y=obj.position
	}else{
		for(var i=0;i<items.length;i++){
			dim_tot+=items[i].w;
			if(items[i].is_hidden){
				dim_tot-=obj.layout_spacing;
			}
		}
		dim_tot=Math.max(dim_tot,0)
		obj.layout_direction="right"
		obj.layout_scroll_x=obj.position
		obj.layout_scroll_y=0
	}
	obj.dim_tot=dim_tot
	obj.selection={};if(obj.value!=undefined){obj.selection['$'+obj.value]=1}
	UI.RoundRect(obj);
	if(!obj.no_region){W.PureRegion(id,obj)}//region goes before children
	if(!obj.no_clipping){UI.PushCliprect(obj.x,obj.y,obj.w,obj.h)}
	if(!obj.dragging_samples){obj.Simulate()}
	//forced clicksel
	var bk_layout_direction=obj.layout_direction
	obj.layout_direction=undefined
	UI.Begin(obj)
		if(!obj.no_region){
			for(var i=0;i<items.length;i++){
				var id_i="$"+i.toString();
				//var obj_item_i=obj[id_i]
				//if(!obj_item_i||obj_item_i.no_click_selection){continue;}
				var id_click_sel="$C"+i.toString();
				//var rx0=obj_item_i.x;
				//var ry0=obj_item_i.y;
				//var rx1=rx0+obj_item_i.w;
				//var ry1=ry0+obj_item_i.h;
				//if(!obj.no_clipping){
				//	rx0=Math.max(rx0,obj.x);
				//	ry0=Math.max(ry0,obj.y);
				//	rx1=Math.min(rx1,obj.x+obj.w);
				//	ry1=Math.min(ry1,obj.y+obj.h);
				//}
				W.Region(id_click_sel,{x:0,y:0,w:0,h:0,
					numerical_id:i,
					__kept:1,
					OnClick:function(event){
						obj.OnChange(this.numerical_id)
						if(obj.is_single_click_mode||event.clicks>1){
							obj["$"+this.numerical_id.toString()].OnDblClick();
						}else{
							UI.SetFocus(obj)
						}
						UI.Refresh()
						return 1;
					},
					OnMouseWheel:obj.OnMouseWheel.bind(obj),
				})
			}
		}
	UI.End("temp")
	//do group after the regions
	obj.layout_direction=bk_layout_direction
	if(UI.enable_timing){
		UI.TimingEvent("starting listview rendering");
	}
	W.PureGroup(obj,"temp")
	obj.layout_direction=undefined
	if(!obj.no_region){
		for(var i=0;i<items.length;i++){
			var id_i="$"+i.toString();
			var obj_item_i=obj[id_i]
			if(!obj_item_i||obj_item_i.no_click_selection){continue;}
			var id_click_sel="$C"+i.toString();
			var rx0=obj_item_i.x;
			var ry0=obj_item_i.y;
			var rx1=rx0+obj_item_i.w;
			var ry1=ry0+obj_item_i.h;
			if(!obj.no_clipping){
				rx0=Math.max(rx0,obj.x);
				ry0=Math.max(ry0,obj.y);
				rx1=Math.min(rx1,obj.x+obj.w);
				ry1=Math.min(ry1,obj.y+obj.h);
			}
			var rgn=obj[id_click_sel];
			if(rgn){
				rgn.x=rx0;
				rgn.y=ry0;
				rgn.w=rx1-rx0;
				rgn.h=ry1-ry0;
			}
		}
	}
	//but before the scrollbars
	UI.Begin(obj)
		if(obj.has_scroll_bar){
			//associated scrollbar
			if(obj.dimension=="y"){
				if(obj.h<dim_tot){
					W.ScrollBar("scroll_bar",{
						anchor:'parent',anchor_align:'right',anchor_valign:'fill',
						dimension:'y',
						x:0,y:0,w:obj.size_scroll_bar,
						value:obj.position/(dim_tot-obj.h),page_size:obj.h,total_size:dim_tot,
						OnMouseWheel:obj.OnMouseWheel.bind(obj),
						OnChange:function(value){
							obj.position=value*(obj.dim_tot-obj.h)
							UI.Refresh()
						}})
				}
			}else{
				if(obj.w<dim_tot){
					W.ScrollBar("scroll_bar",{
						anchor:'parent',anchor_align:'fill',anchor_valign:'down',
						dimension:'x',
						x:0,y:0,h:obj.size_scroll_bar,
						value:obj.position/(dim_tot-obj.w),page_size:obj.w,total_size:dim_tot,
						OnMouseWheel:obj.OnMouseWheel.bind(obj),
						OnChange:function(value){
							obj.position=value*(obj.dim_tot-obj.w)
							UI.Refresh()
						}})
				}
			}
		}
	UI.End()
	if(!obj.no_clipping){UI.PopCliprect()}
	if(id_changed&&items.length>0){
		obj.OnChange(obj.value)
	}
	return obj;
}

W.AnimationNode=function(id,attrs){
	return UI.StdWidget(id,attrs,"animation_node",W.AnimationNode_prototype)
}
