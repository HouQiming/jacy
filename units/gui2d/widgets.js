////////////////////////////////////////
//basic primitives
var UI=require("gui2d/ui");
var W=exports;

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
	UI.SDL_SetTextInputRect(x*UI.pixels_per_unit,y*UI.pixels_per_unit,w*UI.pixels_per_unit,h*UI.pixels_per_unit)
};

W.Window=function(id,attrs){
	attrs=UI.Keep(id,attrs);
	//the dpi is not per-inch,
	if(!UI.pixels_per_unit){
		var display_mode=UI.SDL_GetCurrentDisplayMode();
		var design_screen_dim=attrs.designated_screen_size||Math.min(attrs.w,attrs.h)||1600;
		var screen_dim=Math.min(display_mode.w,display_mode.h);
		UI.pixels_per_unit=screen_dim/design_screen_dim;
		UI.ResetRenderer(UI.pixels_per_unit,attrs.gamma||2.2);
		UI.LoadStaticImages(UI.rc);
		//wipe out initialization routines for security
		UI.LoadPackedTexture=null;
		UI.LoadStaticImages=null;
	}
	if(!attrs.__hwnd){
		//no default event handler for the window
		attrs.__hwnd=UI.SDL_CreateWindow(attrs.title||"untitled",attrs.x||UI.SDL_WINDOWPOS_CENTERED,attrs.y||UI.SDL_WINDOWPOS_CENTERED,attrs.w*UI.pixels_per_unit,attrs.h*UI.pixels_per_unit, attrs.flags);
	}
	//defer the innards painting to the first OnPaint - need the GL context
	UI.context_paint_queue.push(attrs);
	UI.HackAllCallbacks(attrs);
	UI.BeginPaint(attrs.__hwnd,attrs);//EndPaint in UI.End()
	attrs.x=0;
	attrs.y=0;
	if(!UI.is_real){
		UI.sandbox_main_window=attrs.__hwnd;
		UI.Clear(attrs.bgcolor||0xffffffff);
	}
	return attrs;
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
	attrs=UI.Keep(id,attrs,proto);
	UI.StdAnchoring(id,attrs);
	UI.context_regions.push(attrs);
	attrs.region___hwnd=UI.context_window.__hwnd;
	return attrs;
}

////////////////////////////////////////
//utility
W.Group=function(id,attrs0){
	var attrs=UI.Keep(id,attrs0);
	UI.StdAnchoring(id,attrs);
	/////////
	var items=attrs.items||[];
	var item_template=attrs.item_template||{};
	var item_object=attrs.item_object;
	var selection=attrs.selection;
	//layouting: just set layout_direction and layout_spacing
	UI.Begin(attrs);
	for(var i=0;i<items.length;i++){
		var items_i=items[i];
		var obj_temp=Object.create(item_template);
		for(var key in items_i){
			obj_temp[key]=items_i[key];
		}
		if(selection){
			obj_temp.selected=selection[items_i.id];
		}
		item_object(items_i.id,obj_temp).__kept=1;
	}
	UI.End(attrs);
	//delete the non-kept
	for(var key in attrs){
		//check for leading $
		if(key.charCodeAt(0)==0x24){
			var child=attrs[key];
			if(child.__kept){
				child.__kept=0;
			}else{
				delete attrs[key];
			}
		}
	}
	return attrs;
}

//todo: clipping/scrolling/zooming host with phone awareness

////////////////////////////////////////
//widgets
var Button_prototype={
	OnMouseOver:function(){this.mouse_state="over";UI.Refresh();},
	OnMouseOut:function(){this.mouse_state="out";UI.Refresh();},
	OnMouseDown:function(){this.mouse_state="down";UI.Refresh();},
	OnMouseUp:function(){this.mouse_state="over";UI.Refresh();}
};
W.Button=function(id,attrs0){
	//////////////////
	//styling
	var attrs=UI.Keep(id,attrs0,Button_prototype);
	UI.StdStyling(id,attrs,attrs0, "button",attrs.mouse_state||"out");
	//size estimation
	var bmpid=(UI.rc[attrs.icon]||0);
	if(attrs.w_icon){
		attrs.w_bmp=attrs.w_icon;
		attrs.h_bmp=attrs.h_icon;
	}else{
		if(bmpid){
			UI.GetBitmapSize(bmpid,attrs);
		}else{
			attrs.w_bmp=0;
			attrs.h_bmp=0;
		}
	}
	UI.LayoutText(attrs);
	var padding=(attrs.padding||4);
	attrs.w=(attrs0.w||(attrs.w_bmp+attrs.w_text+padding*2));
	attrs.h=(attrs0.h||(Math.max(attrs.h_bmp,attrs.h_text)+padding*2));
	UI.StdAnchoring(id,attrs);
	//////////////////
	//rendering
	UI.RoundRect(attrs);
	var x=attrs.x+padding;
	if(bmpid){UI.DrawBitmap(bmpid,x,attrs.y+(attrs.h-attrs.h_bmp)*0.5,attrs.w_bmp,attrs.h_bmp,attrs.icon_color||0xffffffff);}
	x+=attrs.w_bmp;
	UI.DrawTextControl(attrs,x,attrs.y+(attrs.h-attrs.h_text)*0.5,attrs.text_color||0xffffffff)
	return W.Region(id,attrs);
}

var Edit_prototype={
	//////////
	scale:1,
	scroll_x:0,
	scroll_y:0,
	x_updown:0,
	//////////
	caret_width:2,
	caret_color:0xff000000,
	caret_flicker:500,
	color:0xff000000,
	bgcolor_selection:0xffffe0d0,
	GetHandlerID:function(name){
		return this.ed.m_handler_registration[name];
	},
	AutoScroll:function(mode){
		//'show' 'center' 'center_if_hidden'
		var ccnt0=this.sel0.ccnt;
		var ccnt1=this.sel1.ccnt;
		var ed=this.ed;
		var ed_caret=ed.XYFromCcnt(ccnt1);
		var y_original=this.scroll_y;
		var ccnt_tot=ed.GetTextSize();
		var ytot=ed.XYFromCcnt(ccnt_tot).y+ed.GetCharacterHeightAt(ccnt_tot);
		var wc=UI.GetCharacterAdvance(ed.GetDefaultFont(),' ');
		var hc=ed.GetCharacterHeightAt(ccnt1);
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
		//TestTrigger(KEYCODE_ANY_MOVE)
		//todo: ui animation?
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
		ed.Edit([ccnt0,ccnt1-ccnt0,event.text])
		var lg=Duktape.__byte_length(event.text);
		this.sel0.ccnt=ccnt0+lg;
		this.sel1.ccnt=ccnt0+lg;
		this.AutoScroll("show");
		if(this.OnChange){this.OnChange(this);}
		UI.Refresh()
	},
	OnKeyDown:function(event){
		//todo: global hotkey table - keyed using things like "CTRL+C", if not found, tokenize at +
		//allow multiple keys
		/*
		mouse messages
		*/
		var ed=this.ed;
		var IsKey=UI.IsKey;
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
		//todo: scrolling
		if(0){
		}else if(IsKey(event,["UP"])||IsKey(event,["SHIFT","UP"])){
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			var bk=this.x_updown;
			sel1.ccnt=ed.SeekXY(this.x_updown,ed_caret.y-1.0);
			epilog();
			this.x_updown=bk;
		}else if(IsKey(event,["DOWN"])||IsKey(event,["SHIFT","DOWN"])){
			var hc=ed.GetCharacterHeightAt(sel1.ccnt);
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			var bk=this.x_updown;
			sel1.ccnt=ed.SeekXY(this.x_updown,ed_caret.y+hc);
			epilog();
			this.x_updown=bk;
		}else if(IsKey(event,["LEFT"])||IsKey(event,["SHIFT","LEFT"])){
			var ccnt=sel1.ccnt;
			if(ccnt>0){
				sel1.ccnt=ed.SnapToCharBoundary(ccnt-1,-1);
				epilog();
			}
		}else if(IsKey(event,["RIGHT"])||IsKey(event,["SHIFT","RIGHT"])){
			var ccnt=sel1.ccnt;
			if(ccnt<ed.GetTextSize()){
				sel1.ccnt=ed.SnapToCharBoundary(ccnt+1,1);
				epilog();
			}
		}else if(IsKey(event,["CTRL","LEFT"])||IsKey(event,["CTRL","SHIFT","LEFT"])){
			var ccnt=sel1.ccnt;
			if(ccnt>0){
				sel1.ccnt=ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt-1,-1),-1,"ctrl_lr_stop")
				epilog();
			}
		}else if(IsKey(event,["CTRL","RIGHT"])||IsKey(event,["CTRL","SHIFT","RIGHT"])){
			var ccnt=sel1.ccnt;
			if(ccnt<ed.GetTextSize()){
				sel1.ccnt=ed.MoveToBoundary(ed.SnapToCharBoundary(ccnt+1,1),1,"ctrl_lr_stop")
				epilog();
			}
		}else if(IsKey(event,["BACKSPACE"])||IsKey(event,["DELETE"])){
			var ccnt0=sel0.ccnt;
			var ccnt1=sel1.ccnt;
			if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
			if(ccnt0==ccnt1){
				if(IsKey(event,["BACKSPACE"])){
					if(ccnt0>0){ccnt0=ed.SnapToCharBoundary(ccnt0-1,-1);}
				}else{
					if(ccnt1<ed.GetTextSize()){ccnt1=ed.SnapToCharBoundary(ccnt1+1,1);}
				}
			}
			if(ccnt0<ccnt1){
				ed.Edit([ccnt0,ccnt1-ccnt0,null])
				if(this.OnChange){this.OnChange(this);}
				UI.Refresh();
				return;
			}
		}else if(IsKey(event,["CTRL","HOME"])||IsKey(event,["CTRL","SHIFT","HOME"])){
			sel1.ccnt=0;
			epilog()
		}else if(IsKey(event,["CTRL","END"])||IsKey(event,["CTRL","SHIFT","END"])){
			sel1.ccnt=ed.GetTextSize();
			epilog()
		}else if(IsKey(event,["CTRL","A"])){
			sel0.ccnt=0;
			sel1.ccnt=ed.GetTextSize();
			UI.Refresh();
		}else if(IsKey(event,["RETURN"])||IsKey(event,["RETURN2"])){
			//todo: DOS mode test
			this.OnTextInput({"text":"\n"})
		}else if(IsKey(event,["HOME"])||IsKey(event,["SHIFT","HOME"])){
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			var ccnt_lhome=ed.SeekXY(0,ed_caret.y);
			var ccnt_ehome=ed.MoveToBoundary(ccnt_lhome,1,"space");
			if(sel1.ccnt==ccnt_ehome){
				sel1.ccnt=ccnt_lhome;
			}else{
				sel1.ccnt=ccnt_ehome;
			}
			epilog();
		}else if(IsKey(event,["END"])||IsKey(event,["SHIFT","END"])){
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			sel1.ccnt=ed.SeekXY(1e17,ed_caret.y);
			epilog();
		}else if(IsKey(event,["PAGEUP"])||IsKey(event,["SHIFT","PAGEUP"])){
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			sel1.ccnt=ed.SeekXY(ed_caret.x,ed_caret.y-this.h);
			epilog();
		}else if(IsKey(event,["PAGEDOWN"])||IsKey(event,["SHIFT","PAGEDOWN"])){
			var hc=ed.GetCharacterHeightAt(sel1.ccnt);
			var ed_caret=ed.XYFromCcnt(sel1.ccnt);
			sel1.ccnt=ed.SeekXY(ed_caret.x,ed_caret.y+this.h);
			epilog();
		}else if(IsKey(event,["CTRL","C"])||IsKey(event,["CTRL","INSERT"])){
			var ccnt0=sel0.ccnt;
			var ccnt1=sel1.ccnt;
			if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
			if(ccnt0<ccnt1){
				UI.SDL_SetClipboardText(ed.GetText(ccnt0,ccnt1-ccnt0))
			}
		}else if(IsKey(event,["CTRL","X"])||IsKey(event,["SHIFT","DELETE"])){
			var ccnt0=sel0.ccnt;
			var ccnt1=sel1.ccnt;
			if(ccnt0>ccnt1){var tmp=ccnt0;ccnt0=ccnt1;ccnt1=tmp;}
			if(ccnt0<ccnt1){
				UI.SDL_SetClipboardText(ed.GetText(ccnt0,ccnt1-ccnt0))
				ed.Edit([ccnt0,ccnt1-ccnt0,null])
				if(this.OnChange){this.OnChange(this);}
				UI.Refresh();
				return;
			}
		}else if(IsKey(event,["CTRL","V"])||IsKey(event,["SHIFT","INSERT"])){
			var stext=UI.SDL_GetClipboardText()
			this.OnTextInput({"text":stext})
		}else if(IsKey(event,["CTRL","Z"])||IsKey(event,["ALT","BACKSPACE"])){
			var ret=ed.Undo()
			if(ret&&ret.sz){
				sel0.ccnt=ret.ccnt;
				sel1.ccnt=ret.ccnt+ret.sz;
				this.AutoScroll("center_if_hidden");
			}
			UI.Refresh();
		}else if(IsKey(event,["CTRL","SHIFT","Z"])||IsKey(event,["CTRL","Y"])){
			var ret=ed.Undo("redo")
			if(ret&&ret.sz){
				sel0.ccnt=ret.ccnt;
				sel1.ccnt=ret.ccnt+ret.sz;
				this.AutoScroll("center_if_hidden");
			}
			UI.Refresh();
		}else{
		}
		if(sel0_ccnt!=sel0.ccnt||sel1_ccnt!=sel1.ccnt){
			if(this.OnSelectionChange){this.OnSelectionChange(this);}
		}
	},
};
W.Edit=function(id,attrs0){
	var attrs=UI.Keep(id,attrs0,Edit_prototype);
	UI.StdStyling(id,attrs,attrs0, "edit",attrs.focus_state||"blur");
	UI.StdAnchoring(id,attrs);
	var ed=attrs.ed;
	if(!ed){
		ed=UI.CreateEditor(attrs);
		if(attrs.text){ed.Edit([0,0,attrs.text],1);}
		attrs.sel0=ed.CreateLocator(0,-1);attrs.sel0.undo_tracked=1;
		attrs.sel1=ed.CreateLocator(0,-1);attrs.sel1.undo_tracked=1;
		attrs.ed=ed;
		attrs.sel_hl=ed.CreateHighlight(attrs.sel0,attrs.sel1);
		attrs.sel_hl.color=attrs.bgcolor_selection;
		attrs.sel_hl.invertible=1;
		ed.m_caret_locator=attrs.sel1;
	}
	//todo: scrolling
	var scale=attrs.scale;
	var scroll_x=attrs.scroll_x;
	var scroll_y=attrs.scroll_y;
	ed.Render({x:scroll_x,y:scroll_y,w:attrs.w,h:attrs.h, scr_x:attrs.x,scr_y:attrs.y, scale:scale});
	if(UI.HasFocus(attrs)){
		var ed_caret=ed.XYFromCcnt(attrs.sel1.ccnt);
		var x_caret=attrs.x+(ed_caret.x-scroll_x+ed.m_caret_offset)*scale;
		var y_caret=attrs.y+(ed_caret.y-scroll_y)*scale;
		UI.SetCaret(UI.context_window,
			x_caret,y_caret,
			attrs.caret_width*scale,UI.GetFontHeight(attrs.font)*scale,
			attrs.caret_color,attrs.caret_flicker);
	}
	return attrs;
};

exports.Edit_prototype=Edit_prototype;
