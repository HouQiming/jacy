////////////////////////////////////////
//basic primitives
var UI=require("gui2d/ui");
var W=exports;

//todo: Mac, iOS, Android fonts
UI.font_name="segoeui,arial"
UI.Theme_Minimalistic=function(C){
	UI.current_theme_color=C[0];
	var C_dark=UI.lerp_rgba(C[0],0xff000000,0.15)
	var C_sel=UI.lerp_rgba(C[0],0xffffffff,0.75)
	UI.default_styles={
		"label":{
			font:UI.Font(UI.font_name,24),
			color:0xff000000,
		},
		button:{
			transition_dt:0.1,
			round:16,border_width:3,padding:12,
			font:UI.Font(UI.font_name,24),
			$:{
				out:{
					border_color:C[0],color:0xffffffff,
					icon_color:C[0],
					text_color:C[0],
				},
				over:{
					border_color:C[0],color:C[0],
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
		edit:{
			//animating edit would ruin a lot of object properties
			transition_dt:0,
			bgcolor_selection:C_sel,
		},
		check_button:{
			transition_dt:0.1,
			round:0,border_width:3,padding:12,
			$:{
				out:{
					border_color:0x00ffffff,color:0x00ffffff,
					icon_color:C[0],
					text_color:C[0],
				},
				over:{
					border_color:C[0],color:C[0],
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				down:{
					border_color:C_dark,color:C_dark,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				////////////////////
				checked_out:{
					border_color:C[0],color:0x00ffffff,
					icon_color:C[0],
					text_color:C[0],
				},
				checked_over:{
					border_color:C[0],color:C[0],
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
				checked_down:{
					border_color:C_dark,color:C_dark,
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
			}
		},
		menu_item:{
			font:UI.Font(UI.font_name,24),
			transition_dt:0.1,
			round:0,padding:8,
			icon_color:0xff000000,
			text_color:0xff000000,
			color:0x00ffffff,
			$:{
				over:{
					color:C[0],
					icon_color:0xffffffff,
					text_color:0xffffffff,
				},
			},
		},
		menu:{
			transition_dt:0.1,
			round:4,border_width:2,padding:8,
			layout_spacing:0,
			border_color:C[0],color:0xffffffff,
		},
		combobox:{
			transition_dt:0.1,
			round:4,border_width:2,padding:8,
			layout_spacing:0,
			border_color:C[0],icon_color:C[0],text_color:0xff000000,color:0xffffffff,icon_text_align:'left',
			font:UI.Font(UI.font_name,24),
			label_font:UI.Font(UI.font_name,24),
		},
		sub_window:{
			transition_dt:0.1,
			round:0,border_width:2,
			padding:4,h_caption:24,
			/////////////////
			layout_direction:"inside",layout_align:'left',layout_valign:'up',
			/////////////////
			font:UI.Font(UI.font_name,20,100),
			color:0xffffffff,border_color:C[0],border_width:2,
			caption_color:C[0],text_color:0xffdddddd,
			button_style:{
				transition_dt:0.1,
				round:0,border_width:2,padding:8,
				border_width:0,color:0,
				text_color:0xffdddddd,
				font:UI.Font(UI.font_name,20,100),
				$:{
					out:{
						text_color:0xffdddddd
					},
					over:{
						text_color:0xffffffff,
					},
					down:{
						text_color:0xffffffff,
					},
				}
			},
		},
		tab_label:{
			transition_dt:0.1,
			shadow_size:8,
			font:UI.Font(UI.font_name,24), padding:16,
			$:{
				active:{
					text_color:0xffffffff,
					color:C[0],
					shadow_color:0xaa000000, 
				},
				inactive:{
					text_color:0xff444444,
					color:C[0]&0x00ffffff,
					shadow_color:0x00000000, 
				},
			}
		},
		tabbed_document:{
			transition_dt:0.1,
			h_caption:32, h_bar:4, color:0xffbbbbbb, border_color:C[0]
		},
		box_document:{
			border_color:(0xcc000000&C[0]),border_width:2,
			color:(0x44000000&C[0]),
		},
		txtx_editor:{
			border_color:0xff000000,border_width:2,
			color:0xffffffff,
		},
		slider:{
			transition_dt:0.1,
			bgcolor:[{x:0,y:0,color:0xffbbbbbb},{x:0,y:1,color:0xffdddddd}],
			//border_width:2, border_color:0xff444444,
			h_slider:8,
			round:8,
			color:C[0],
			padding:0,
			//label_text:'▲',
			//label_raise:0.4,
			//label_font:UI.Font(UI.font_name,32),
			//label_color:C[0],
			middle_bar:{
				w:8,h:8,
				round:2,
				color:0xffffffff, border_width:2, border_color:0xff444444,
			},
		},
		edit_box:{
			transition_dt:0.1,
			round:4,padding:8,
			color:0xffffffff,
			border_width:2,
			border_color:0xffbbbbbb,
			font:UI.Font(UI.font_name,24),
			text_color:0xff000000,
			$:{
				blur:{
					border_color:0xffbbbbbb,
				},
				focus:{
					border_color:C[0],
				},
			},
		},
		color_picker:{
			w_text:16,w_slider:128,w_edit:54,
			h_slider:12,
			h_edit:32,
			h_space:24,
			padding:8,
			border_width:1.5,
			border_color:0xff444444,
			text_color:0xff000000,
			font:UI.Font(UI.font_name,24),
		},
		select:{
			transition_dt:0.1,
			value_animated:0,
			font:UI.Font(UI.font_name,24),
			padding:12,spacing:12,
			combo_box_padding:40,
			slider_style:{
				transition_dt:0.1,
				tolerance:2,
				w:32,h_slider:8,
				bgcolor:[{x:0,y:0,color:0xffbbbbbb},{x:0,y:1,color:0xffdddddd}],
				round:8,
				color:C[0],
				padding:0,
				middle_bar:{
					w:16,h:8,
					round:8,
					color:0xffffffff, border_width:2, border_color:0xff444444,
				},
			},
			button_style:{
				transition_dt:0.1,
				color:0xffffffff,border_color:C[0],
				round:16,border_width:2,padding:12,
				$:{
					out:{
						text_color:C[0],
					},
					over:{
						text_color:C_dark,
					},
					down:{
						text_color:C_dark,
					},
					////////////////////
					checked_out:{
						color:C[0],
						text_color:0xffffffff,
					},
					checked_over:{
						color:C[0],
						text_color:0xffffffff,
					},
					checked_down:{
						color:C[0],
						text_color:0xffffffff,
					},
				}
			},
		}
	};
};

UI.DestroyWindow=function(attrs){
	UI.CallIfAvailable(attrs,"OnDestroy");
	if(attrs.is_main_window){
		UI.SDL_PostQuitEvent();
	}
	UI.SDL_DestroyWindow(attrs.__hwnd)
};

UI.SetCaret=function(attrs,x,y,w,h,C,dt){
	attrs.caret_x=x;
	attrs.caret_y=y;
	attrs.caret_w=w;
	attrs.caret_h=h;
	attrs.caret_C=C;
	attrs.caret_state=1;
	attrs.caret_dt=dt;
	attrs.caret_is_set=1
	UI.SDL_SetTextInputRect(x*UI.pixels_per_unit,y*UI.pixels_per_unit,w*UI.pixels_per_unit,h*UI.pixels_per_unit)
};

UI.ChooseScalingFactor=function(obj){
	var display_mode=UI.SDL_GetCurrentDisplayMode();
	var design_screen_dim=obj.designated_screen_size||Math.min(obj.w,obj.h)||1600;
	var screen_dim=Math.min(display_mode.w,display_mode.h);
	UI.pixels_per_unit=screen_dim/design_screen_dim;
	UI.ResetRenderer(UI.pixels_per_unit,obj.gamma||2.2);
	UI.LoadStaticImages(UI.rc);
	////wipe out initialization routines for security
	//UI.LoadPackedTexture=null;
	//UI.LoadStaticImages=null;
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
	}
	//defer the innards painting to the first OnPaint - need the GL context
	UI.context_paint_queue.push(obj);
	UI.BeginPaint(obj.__hwnd,obj);//EndPaint in UI.End()
	obj.x=0;
	obj.y=0;
	obj.caret_is_set=0
	if(!UI.is_real){
		UI.sandbox_main_window=obj.__hwnd;
		UI.sandbox_main_window_w=obj.w*UI.pixels_per_unit;
		UI.sandbox_main_window_h=obj.h*UI.pixels_per_unit;
		UI.Clear(obj.bgcolor||0xffffffff);
	}
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
	if(!attrs.__layout){UI.LayoutText(attrs);}
	attrs.w=(attrs.w||attrs.w_text);
	attrs.h=(attrs.h||attrs.h_text);
	UI.StdAnchoring(id,attrs);
	UI.DrawTextControl(attrs,attrs.x,attrs.y,attrs.color||0xffffffff)
	return attrs
};

W.RoundRect=function(id,attrs){
	UI.StdAnchoring(id,attrs);
	UI.RoundRect(attrs)
	return attrs;
}

UI.DrawIcon=function(attrs){
	//bmp or ttf
	//todo: height-sized?
	var bmpid;
	var icon_font,icon_char;
	var size={};
	if(attrs.icon_file){
		bmpid=UI.rc[attrs.icon_file];
		UI.GetBitmapSize(bmpid,size);
	}else{
		icon_font=attrs.icon_font;
		icon_char=attrs.icon_char.charCodeAt(0);
		size.w=UI.GetCharacterAdvance(icon_font,icon_char);
		size.h=UI.GetCharacterHeight(icon_font);
	}
	var w=attrs.w||size.w;
	var h=attrs.h||size.h;
	var x=attrs.x+(w-size.w)*0.5;
	var y=attrs.y+(h-size.h)*0.5;
	if(attrs.icon_file){
		UI.DrawBitmap(0,x,y,size.w,size.h,attrs.color);
	}else{
		//todo: character border...
		UI.DrawChar(icon_font,x,y,icon_char)
	}
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
	UI.HackCallback(attrs.action);
	UI.context_hotkeys.push(attrs);
	return attrs;
}

W.Region=function(id,attrs,proto){
	//attrs is needed to track OnClick and stuff, *even if we don't store any var*
	var obj=UI.Keep(id,attrs,proto);
	UI.StdAnchoring(id,obj);
	return W.PureRegion(id,obj)
}

W.PureRegion=function(id,obj){
	if(obj==UI.nd_focus){
		UI.context_focus_is_a_region=1
	}
	UI.context_regions.push(obj);
	obj.region___hwnd=UI.context_window.__hwnd;
	return obj;
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
	/////////
	var items=obj.items||[];
	var item_template=obj.item_template||{};
	var selection=obj.selection;
	var sel_obj_temps;
	//layouting: just set layout_direction and layout_spacing
	UI.Begin(obj);
	obj.layout_auto_anchor=null;
	for(var i=0;i<items.length;i++){
		var items_i=items[i];
		if(!items_i.id){items_i.id="$"+i.toString();}
		if(items_i.is_hidden){continue;}
		var itemobj_i=obj[items_i.id];
		if(itemobj_i&&itemobj_i.is_hidden){itemobj_i.__kept=1;continue;}
		var obj_temp=Object.create(item_template);
		obj_temp.x=0;obj_temp.y=0;//for layouting
		for(var key in items_i){
			obj_temp[key]=items_i[key];
		}
		if(selection){
			obj_temp.selected=selection[obj_temp.id];
		}
		itemobj_i=(obj_temp.object_type)(obj_temp.id,obj_temp);
		itemobj_i.__kept=1;
	}
	obj.layout_auto_anchor=null;
	UI.End(obj);
	//delete the non-kept
	for(var key in obj){
		//check for leading $
		if(key.charCodeAt(0)==0x24){
			var child=obj[key];
			if(child.__kept){
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
	var bmpid=(UI.rc[obj.icon]||0);
	if(obj.w_icon){
		obj.w_bmp=obj.w_icon;
		obj.h_bmp=obj.h_icon;
	}else{
		if(bmpid){
			UI.GetBitmapSize(bmpid,obj);
		}else{
			obj.w_bmp=0;
			obj.h_bmp=0;
		}
	}
	//obj.w=1e17;//for UI.LayoutText
	//UI.LayoutText(obj);
	var text_dim=UI.MeasureText(obj.font,obj.text)
	var padding=(obj.padding||0);
	return {w:(obj.w_bmp+text_dim.w+padding*2),h:Math.max(obj.h_bmp,text_dim.h)+padding*2};
}

W.DrawIconText=function(id,obj,attrs){
	//size estimation
	var bmpid=(UI.rc[obj.icon]||0);
	if(obj.w_icon){
		obj.w_bmp=obj.w_icon;
		obj.h_bmp=obj.h_icon;
	}else{
		if(bmpid){
			UI.GetBitmapSize(bmpid,obj);
		}else{
			obj.w_bmp=0;
			obj.h_bmp=0;
		}
	}
	obj.w=1e17;//for UI.LayoutText
	UI.LayoutText(obj);
	var padding=(obj.padding||0);
	if(obj.use_measured_dims){
		obj.w=(attrs.w||(obj.w_bmp+obj.w_text+padding*2));
		obj.h=(attrs.h||(Math.max(obj.h_bmp,obj.h_text)+padding*2));
		UI.StdAnchoring(id,obj);
	}
	//print(obj.w,obj.h,obj.w_bmp,obj.h_bmp,obj.w_text,obj.h_text)
	//////////////////
	//rendering
	UI.RoundRect(obj);
	var alg=(obj.icon_text_align||'center');
	var inner_w=obj.w_bmp+obj.w_text;
	var x;
	if(alg=='left'){
		x=obj.x+padding;
	}else if(alg=='center'){
		x=obj.x+(obj.w-inner_w)*0.5;
	}else{
		x=obj.x+(obj.w-inner_w-padding);
	}
	var compute_y=function(inner_h){
		var alg=(obj.icon_text_valign||'center');
		if(alg=='up'){
			return obj.y+padding;
		}else if(alg=='center'){
			return obj.y+(obj.h-inner_h)*0.5;
		}else{
			return obj.y+(obj.h-inner_h-padding);
		}
	}
	if(bmpid){UI.DrawBitmap(bmpid,x,compute_y(obj.h_bmp),obj.w_bmp,obj.h_bmp,obj.icon_color||0xffffffff);}
	x+=obj.w_bmp;
	UI.DrawTextControl(obj,x,compute_y(obj.h_text),obj.text_color||0xffffffff)
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

W.Button=function(id,attrs){
	//////////////////
	//styling
	var obj=UI.Keep(id,attrs,W.Button_prototype);
	UI.StdStyling(id,obj,attrs, "button",(obj.value?"checked_":"")+(obj.mouse_state||"out"));
	W.DrawIconText(id,obj,attrs)
	return W.PureRegion(id,obj);
}

W.Edit_prototype={
	//////////
	scale:1,
	scroll_x:0,
	scroll_y:0,
	x_updown:0,
	//////////
	mouse_cursor:"ibeam",
	default_style_name:"edit",
	caret_width:2,
	caret_color:0xff000000,
	caret_flicker:500,
	color:0xff000000,
	bgcolor_selection:0xffffe0d0,
	caret_is_wrapped:0,
	GetHandlerID:function(name){
		return this.ed.m_handler_registration[name];
	},
	GetCharacterHeightAtCaret:function(){
		var ccnt1=this.sel1.ccnt;
		if(this.caret_is_wrapped){
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
		var wc=UI.GetCharacterAdvance(ed.GetDefaultFont(),' ');
		var hc=this.GetCharacterHeightAtCaret();
		var page_height=this.h;
		if(mode!='show'){
			if(this.scroll_y>ed_caret.y||this.scroll_y<=ed_caret.y-page_height||mode=='center'){
				//mode 'center_if_hidden': only center when the thing was invisible
				this.scroll_y=ed_caret.y-(page_height-hc)/2;
			}
		}
		this.scroll_y=Math.min(this.scroll_y,ed_caret.y)
		if(ed_caret.y-this.scroll_y>=page_height-hc){
			this.scroll_y=(ed_caret.y-(page_height-hc));
		}
		var wwidth=this.w-wc;
		if(mode!='show'&&this.sel0.ccnt!=this.sel1.ccnt){
			//make sure sel0 shows up
			this.scroll_x=Math.min(this.scroll_x,Math.min(ed.XYFromCcnt(ccnt0).x,ed_caret.x))
		}
		this.scroll_x=Math.min(this.scroll_x,ed_caret.x);
		//if .do_wrap&&this.scroll_x&&ed_caret.x<wwidth:
		//	this.scroll_x=0
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
		this.scroll_y=Math.max(Math.min(this.scroll_y,ytot-(page_height-hc)),0);
		this.x_updown=ed_caret.x
		if(this.disable_scrolling_x){this.scroll_x=0;}
		if(this.disable_scrolling_y){this.scroll_y=0;}
		//TestTrigger(KEYCODE_ANY_MOVE)
		//todo: ui animation?
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
	GetLC:function(ccnt){
		var ed=this.ed;
		return ed.GetStateAt(ed.m_handler_registration["line_column"],ccnt,"ll");
	},
	GetEnhancedHome:function(ccnt){
		var ccnt_lhome=this.SeekLC(this.GetLC(ccnt)[0],0);
		return this.SnapToValidLocation(this.ed.MoveToBoundary(ccnt_lhome,1,"space"),1)
	},
	GetEnhancedEnd:function(ccnt){
		var ccnt_lend=this.SeekLC(this.GetLC(ccnt)[0],1e17);
		if(ccnt_lend>0&&this.ed.GetText(ccnt_lend-1,1)=="\n"){ccnt_lend--;}
		return this.SnapToValidLocation(this.ed.MoveToBoundary(ccnt_lend,-1,"space"),-1)
	},
	////////////////////////////
	GetCaretXY:function(){
		var ed=this.ed;
		var xy0=ed.XYFromCcnt(this.sel1.ccnt)
		if(this.caret_is_wrapped){
			//chicken-egg: no this.GetCharacterHeightAtCaret() here
			xy0.x=0
			xy0.y+=ed.GetCharacterHeightAt(this.sel1.ccnt);
		}
		return xy0;
	},
	SeekXY:function(x,y){
		var ed=this.ed
		var ccnt=ed.SeekXY(x,y)
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
		if(ed.IsAtLineWrap(this.sel1.ccnt)){
			var x0=this.GetCaretXY().x;
			this.caret_is_wrapped=1;
			var x1=this.GetCaretXY().x;
			this.caret_is_wrapped=(Math.abs(x1-x)<Math.abs(x0-x)?1:0);
		}
	},
	CallOnChange:function(){
		this.caret_is_wrapped=0;
		this.AutoScroll('show')
		if(this.OnChange){this.OnChange(this);}
	},
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
		if(this.is_single_line){
			stext=stext.replace("\n"," ");
		}
		this.OnTextInput({"text":stext})
	},
	////////////////////////////
	HookedEdit:function(ops){this.ed.Edit(ops);},
	GetSelection:function(){
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
		return [ccnt0,ccnt1];
	},
	Init:function(){
		var ed=this.ed;
		if(!ed){
			ed=UI.CreateEditor(this);
			if(this.text){ed.Edit([0,0,this.text],1);}
			this.sel0=ed.CreateLocator(0,-1);this.sel0.undo_tracked=1;
			this.sel1=ed.CreateLocator(0,-1);this.sel1.undo_tracked=1;
			this.ed=ed;
			this.sel_hl=ed.CreateHighlight(this.sel0,this.sel1);
			this.sel_hl.color=this.bgcolor_selection;
			this.sel_hl.invertible=1;
			ed.m_caret_locator=this.sel1;
		}
	},
	OnTextEdit:function(event){
		this.ed.m_IME_overlay=event;
		UI.Refresh()
	},
	OnTextInput:function(event){
		var ed=this.ed;
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		if(ccnt0>ccnt1){var tmp=ccnt1;ccnt1=ccnt0;ccnt0=tmp;}
		var ops=[ccnt0,ccnt1-ccnt0,event.text];
		this.HookedEdit(ops)
		//for hooked case, need to recompute those
		var lg=Duktape.__byte_length(ops[2]);
		this.sel0.ccnt=ops[0]+lg;
		this.sel1.ccnt=ops[0]+lg;
		this.AutoScroll("show");
		this.CallOnChange();
		UI.Refresh()
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
		var epilog=function(){
			if(!is_shift){sel0.ccnt=sel1.ccnt;}
			this_outer.AutoScroll("show");
			UI.Refresh();
		};
		if(this.additional_hotkeys){
			var hk=this.additional_hotkeys;
			for(var i=0;i<hk.length;i++){
				if(IsHotkey(event,hk[i].key)){
					hk[i].action.call(this);
					return;
				}
			}
		}
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
			if(this.caret_is_wrapped){
				this.caret_is_wrapped=0;
				epilog();
			}else{
				if(ccnt>0){
					sel1.ccnt=this.SnapToValidLocation(ccnt-1,-1);
					epilog();
				}
			}
		}else if(IsHotkey(event,"RIGHT SHIFT+RIGHT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,1);
			if(!this.caret_is_wrapped&&ed.IsAtLineWrap(ccnt)){
				this.caret_is_wrapped=1;
				epilog();
			}else{
				if(ccnt<ed.GetTextSize()){
					sel1.ccnt=this.SnapToValidLocation(ccnt+1,1);
					epilog();
				}
				this.caret_is_wrapped=0;
			}
		}else if(IsHotkey(event,"CTRL+LEFT CTRL+SHIFT+LEFT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,-1);
			if(ccnt>0){
				sel1.ccnt=this.SnapToValidLocation(ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt-1,-1),-1,"ctrl_lr_stop"),-1)
				this.caret_is_wrapped=(ed.IsAtLineWrap(sel1.ccnt)?1:0);
				epilog();
			}
		}else if(IsHotkey(event,"CTRL+RIGHT CTRL+SHIFT+RIGHT")){
			var ccnt=this.SkipInvisibles(sel1.ccnt,1);
			if(ccnt<ed.GetTextSize()){
				sel1.ccnt=this.SnapToValidLocation(ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt+1,1),1,"ctrl_lr_stop"),1)
				this.caret_is_wrapped=0;
				epilog();
			}
		}else if(IsHotkey(event,"BACKSPACE")||IsHotkey(event,"DELETE")){
			var ccnt0=sel0.ccnt;
			var ccnt1=sel1.ccnt;
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
		}else if(IsHotkey(event,"HOME SHIFT+HOME")){
			var ed_caret=this.GetCaretXY();
			var ccnt_lhome=this.SeekXY(0,ed_caret.y);
			var ccnt_ehome=Math.max(this.GetEnhancedHome(sel1_ccnt),ccnt_lhome);
			if(sel1.ccnt==ccnt_ehome||ccnt_lhome==ccnt_ehome){
				sel1.ccnt=ccnt_lhome;
				this.caret_is_wrapped=(ed.IsAtLineWrap(this.sel1.ccnt)?1:0);
			}else{
				sel1.ccnt=ccnt_ehome;
				this.caret_is_wrapped=0;
			}
			epilog();
		}else if(IsHotkey(event,"END SHIFT+END")){
			var ed_caret=this.GetCaretXY();
			var ccnt_lend=this.SeekXY(1e17,ed_caret.y);
			var ccnt_eend=Math.min(this.GetEnhancedEnd(sel1_ccnt),ccnt_lend);
			if(sel1.ccnt==ccnt_eend||ccnt_lend==ccnt_eend){
				sel1.ccnt=ccnt_lend;
				this.caret_is_wrapped=0;
			}else{
				sel1.ccnt=ccnt_eend;
				this.caret_is_wrapped=0;
			}
			epilog();
		}else if(IsHotkey(event,"PGUP SHIFT+PGUP")){
			var ed_caret=this.GetCaretXY();
			this.MoveCursorToXY(ed_caret.x,ed_caret.y-this.h);
			epilog();
		}else if(IsHotkey(event,"PGDN SHIFT+PGDN")){
			var ed_caret=this.GetCaretXY();
			this.MoveCursorToXY(ed_caret.x,ed_caret.y+this.h);
			epilog();
		}else if(IsHotkey(event,"CTRL+C")||IsHotkey(event,"CTRL+INSERT")){
			this.Copy()
		}else if(IsHotkey(event,"CTRL+X")||IsHotkey(event,"SHIFT+DELETE")){
			if(this.Cut()){return;}
		}else if(IsHotkey(event,"CTRL+V")||IsHotkey(event,"SHIFT+INSERT")){
			this.Paste()
		}else if(IsHotkey(event,"CTRL+Z")||IsHotkey(event,"ALT+BACKSPACE")){
			var ret=ed.Undo()
			if(ret&&ret.sz){
				sel0.ccnt=ret.ccnt;
				sel1.ccnt=ret.ccnt+ret.sz;
				this.AutoScroll("center_if_hidden");
			}
			this.CallOnChange();
			UI.Refresh();
		}else if(IsHotkey(event,"CTRL+SHIFT+Z")||IsHotkey(event,"CTRL+Y")){
			var ret=ed.Undo("redo")
			if(ret&&ret.sz){
				sel0.ccnt=ret.ccnt;
				sel1.ccnt=ret.ccnt+ret.sz;
				this.AutoScroll("center_if_hidden");
			}
			this.CallOnChange();
			UI.Refresh();
		}else{
		}
		if(sel0_ccnt!=sel0.ccnt||sel1_ccnt!=sel1.ccnt){
			if(this.OnSelectionChange){this.OnSelectionChange(this);}
		}
	},
	////////////////////////////
	OnMouseDown:function(event){
		var x0=event.x-this.x+this.scroll_x
		var y0=event.y-this.y+this.scroll_y
		var ccnt_clicked=this.SeekXY(x0,y0);
		if(event.clicks==2){
			//double-click
			this.sel0.ccnt=this.SnapToValidLocation(this.ed.MoveToBoundary(this.ed.SnapToCharBoundary(Math.max(this.SkipInvisibles(ccnt_clicked,-1)-1,0),-1),-1,"word_boundary"),-1)
			this.sel1.ccnt=this.SnapToValidLocation(this.ed.MoveToBoundary(this.ed.SnapToCharBoundary(Math.min(this.SkipInvisibles(ccnt_clicked,1)+1,this.ed.GetTextSize()),1),1,"word_boundary"),1)
			UI.Refresh()
			return
		}
		if(event.clicks>=3){
			//triple-click
			var line=this.GetLC(ccnt_clicked)[0]
			this.sel0.ccnt=this.SeekLC(line,0)
			this.sel1.ccnt=this.SeekLC(line+1,0)
			UI.Refresh()
			return
		}
		this.is_dragging=1
		this.sel0.ccnt=ccnt_clicked;
		this.sel1.ccnt=ccnt_clicked;
		UI.SetFocus(this)
		UI.CaptureMouse(this)
		if(this.OnSelectionChange){this.OnSelectionChange(this);}
		UI.Refresh()
	},
	OnMouseMove:function(event){
		if(!this.is_dragging){return;}
		var x1=event.x-this.x+this.scroll_x
		var y1=event.y-this.y+this.scroll_y
		this.sel1.ccnt=this.SeekXY(x1,y1);
		if(this.OnSelectionChange){this.OnSelectionChange(this);}
		UI.Refresh()
	},
	OnMouseUp:function(event){
		if(!this.is_dragging){return;}
		UI.ReleaseMouse(this)
		this.is_dragging=0
		UI.Refresh()
	},
};
W.Edit=function(id,attrs,proto){
	var obj=UI.Keep(id,attrs,proto||W.Edit_prototype);
	UI.StdStyling(id,obj,attrs, obj.default_style_name,UI.HasFocus(obj)?"focus":"blur");
	UI.StdAnchoring(id,obj);
	if(obj.show_background){
		UI.DrawBitmap(0,obj.x,obj.y,obj.w,obj.h,obj.bgcolor);
	}
	if(!obj.ed){obj.Init()}
	var scale=obj.scale;
	var scroll_x=obj.scroll_x;
	var scroll_y=obj.scroll_y;
	var ed=obj.ed;
	//todo: hint_text for empty box
	ed.Render({x:scroll_x,y:scroll_y,w:obj.w/scale,h:obj.h/scale, scr_x:obj.x,scr_y:obj.y, scale:scale});
	if(UI.HasFocus(obj)){
		var ed_caret=obj.GetCaretXY();
		var x_caret=obj.x+(ed_caret.x-scroll_x+ed.m_caret_offset)*scale;
		var y_caret=obj.y+(ed_caret.y-scroll_y)*scale;
		UI.SetCaret(UI.context_window,
			x_caret,y_caret,
			obj.caret_width*scale,obj.GetCharacterHeightAtCaret()*scale,
			obj.caret_color,obj.caret_flicker);
	}
	return W.PureRegion(id,obj);
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
	if(UI.HasFocus(obj)){
		//auto width/height, drag-scrolling
		if(!obj.selection){
			obj.selection={"$0":1};
		}
		UI.RoundRect(obj);
		W.PureRegion(id,obj)//region goes before children
		W.Group(id,attrs)
		if(!attrs.w||!attrs.h){
			//auto sizing
			var w_max=0;
			var h_tot=0;
			var spacing=(obj.layout_spacing||0);
			for(var i=0;i<obj.items.length;i++){
				var item_i=obj[obj.items[i].id];
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
	}else{
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
	UI.End(obj)
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
		obj.EndContinuousChange()
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
		var tmp={w:1e17,h:1e17,font:obj.label_font,text:obj.label_text}
		UI.LayoutText(tmp);
		UI.Begin(obj)
			var obj_label={}
			obj_label.x=obj.x+w_value-tmp.w_text*0.5
			obj_label.y=obj.y+obj.h-tmp.h_text*obj.label_raise
			obj_label.w=tmp.w_text
			obj_label.h=tmp.h_text
			obj_label.parent=obj
			W.Region("label",obj_label,W.SliderLabel_prototype)
			UI.DrawTextControl(tmp,obj_label.x,obj_label.y,obj.label_color)
		UI.End()
	}
	return obj;
}

W.EditBox_prototype={
	value:"",
	focus_state:"blur",
	OnClick:function(){
		if(this.focus_state=="blur"){
			this.bak_value=this.value;
			this.focus_state="focus"
		}else{
			this.focus_state="blur"
		}
		UI.Refresh()
	},
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
	var dim_text=UI.MeasureIconText({font:obj.font,text:obj.hint_text||obj.value||"0"})
	UI.Begin(obj)
		if(obj.focus_state=="focus"){
			var is_newly_created=!obj.edit;
			W.Edit("edit",{
				anchor:'parent',anchor_align:"left",anchor_valign:"center",
				x:obj.padding,y:0,w:obj.w-obj.padding*2,h:dim_text.h,
				font:obj.font, color:obj.text_color, text:obj.value,
				hint_text:obj.hint_text,
				is_single_line:1,
				additional_hotkeys:[{key:"ESCAPE",action:function(){
					//cancel the change
					obj.OnChange(obj.bak_value)
					obj.bak_value=undefined
					obj.focus_state="blur"
					UI.Refresh()
				}}],
				OnBlur:function(){
					obj.OnChange(obj.edit.ed.GetText())
					obj.focus_state="blur"
					UI.Refresh()
				},
				OnEnter:function(){
					this.OnBlur()
				},
			});
			if(is_newly_created){
				UI.SetFocus(obj.edit)
				obj.edit.sel0.ccnt=0
				obj.edit.sel1.ccnt=obj.edit.ed.GetTextSize()
			}
		}else{
			//text
			obj.edit=undefined
			W.Text("text",{
				anchor:'parent',anchor_align:"left",anchor_valign:"center",
				x:obj.padding,y:0,w:dim_text.w,h:dim_text.h,
				font:obj.font, color:obj.text_color, text:obj.value,
				});
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
				for(var i=0;i<obj.items.length;i++){
					items2[i]={'text':obj.items[i]};
				}
				W.ComboBox("combobox",{
					x:obj.x+obj.w-w_max,y:obj.y,w:w_max,h:obj.h,
					items:items2,value:items[obj.value],
					style:obj.combo_box_style,
					OnChange:function(value){
						for(var i=0;i<obj.items.length;i++){
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
	anchor:'parent',
	anchor_placement:'right',
	position:0,velocity:0,max_velocity:1,acceleration:0.2,
	Simulate:function(){
		var a=this.dragging_samples;
		if(a){
			var n=a.length,p0=Math.max(n-3,0);
			this.position=a[n-1]-a[0]+this.initial_position
			if(n-1<=p0){
				//don't even change the velocity yet
			}else{
				this.velocity=(a[n-1]-a[p0])/(n-1-p0)
			}
			if(this.position<w*0.5){
				this.target_position=0
			}else{
				this.target_position=w
			}
		}else{
			if(this.target_position){
				this.position+=this.velocity;
				if(this.target_position>this.position){
					this.velocity+=this.acceleration;
				}else{
					this.velocity-=this.acceleration;
				}
				if(this.target_position==this.position){
					this.velocity=0
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
		obj.initial_position=obj.position
		this.OnMouseMove(event);
		obj.target_position=(obj.position==0?obj.w:0)
	},
	OnMouseMove:function(event){
		var obj=this.owner;
		if(!obj.dragging_samples){return;}
		if(obj.anchor_placement=='left'||obj.anchor_placement=='top'){
			obj.dragging_samples.push(-event.x)
		}else{
			obj.dragging_samples.push(event.x)
		}
		obj.Simulate()
		UI.Refresh()
	},
	OnMouseUp:function(event){
		var obj=this.owner;
		obj.dragging_samples=null
		UI.Refresh()
	}
}
var g_inverse_dir={'left':'right','right':'left','up':'down','down':'up','center':'center','fill':'fill'}
W.AutoHidePanel=function(id,attrs){
	var obj=UI.Keep(id,attrs,W.AutoHidePanel_prototype)
	UI.StdStyling(id,obj,attrs,"auto_hide_panel")
	if(obj.anchor_placement=='left'||obj.anchor_placement=='right'){
		obj.x=-obj.position;obj.y=0
		obj.anchor_align=obj.anchor_placement
		obj.anchor_valign='fill'
	}else{
		obj.y=-obj.position;obj.x=0
		obj.anchor_valign=obj.anchor_placement
		obj.anchor_align='fill'
	}
	UI.StdAnchoring(id,obj)
	//simply place the child object inside Begin / End
	UI.Begin(obj)
		//one of w/h will be overwriten with fill anyway
		W.Region("knob",{
			anchor:'parent',anchor_placement:g_inverse_dir[obj.anchor_placement],anchor_align:obj.anchor_align,anchor_valign:obj.anchor_valign,
			x:0,y:0,w:obj.knob_size,h:obj.knob_size,
			owner:obj
		},W.AutoHidePanel_knob_prototype)
		if(!obj.dragging_samples){obj.Simulate()}
	UI.End()
	return obj;
}

//todo: ListView - oob-scrolling handlers, default item template, keyboard operation, editbox - long-press selection
