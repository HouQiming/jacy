////////////////////////////////////////
//basic primitives
var UI=require("gui2d/ui").UI;
W={};

UI.DestroyWindow=function(attrs){
	UI.CallIfAvailable(attrs,"OnDestroy",attrs);
	if(attrs.is_main_window){
		UI.SDL_PostQuitEvent();
	}
	UI.SDL_DestroyWindow(attrs.hwnd)
};

UI.SetCaret=function(attrs,x,y,w,h,C,dt){
	attrs.caret_x=x;
	attrs.caret_y=y;
	attrs.caret_w=w;
	attrs.caret_h=h;
	attrs.caret_C=C;
	attrs.caret_state=1;
	attrs.caret_dt=dt;
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
	if(!attrs.hwnd){
		//no default event handler for the window
		attrs.hwnd=UI.SDL_CreateWindow(attrs.title||"untitled",attrs.x||UI.SDL_WINDOWPOS_CENTERED,attrs.y||UI.SDL_WINDOWPOS_CENTERED,attrs.w*UI.pixels_per_unit,attrs.h*UI.pixels_per_unit, attrs.flags);
	}
	//defer the innards painting to the first OnPaint - need the GL context
	attrs.bgcolor=(attrs.bgcolor)
	UI.context_paint_queue.push(attrs);
	UI.HackAllCallbacks(attrs);
	if(UI.context_window_painting){UI.EndPaint();}
	UI.BeginPaint(attrs.hwnd,attrs);
	UI.context_window_painting=1;
	attrs.x=0;
	attrs.y=0;
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
}

W.RoundRect=function(id,attrs){
	UI.StdAnchoring(id,attrs);
	UI.RoundRect(attrs)
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

W.Region=function(id,attrs){
	//attrs is needed to track OnClick and stuff, *even if we don't store any var*
	attrs=UI.Keep(id,attrs);
	UI.StdAnchoring(id,attrs);
	UI.context_regions.push(attrs);
	return attrs
}

////////////////////////////////////////
//widgets
W.Button=function(id,attrs0){
	//////////////////
	//styling
	var attrs=UI.Keep(id,attrs0);
	UI.StdStyling(id,attrs,attrs0, "button",attrs.mouse_state||"out");
	//size estimation
	var bmpid=(UI.rc[attrs.icon]||0);
	if(attrs.w_icon){
		attrs.w_bmp=attrs.w_icon;
		attrs.h_bmp=attrs.h_icon;
	}else{
		UI.GetBitmapSize(bmpid,attrs);
	}
	UI.LayoutText(attrs);
	var padding=(attrs.padding||4);
	attrs.w=(attrs0.w||(attrs.w_bmp+attrs.w_text+padding*2));
	attrs.h=(attrs0.h||(Math.max(attrs.h_bmp,attrs.h_text)+padding*2));
	UI.StdAnchoring(id,attrs);
	//////////////////
	//standard events
	attrs.OnMouseOver=(attrs.OnMouseOver||function(){attrs.mouse_state="over";UI.Refresh();});
	attrs.OnMouseOut=(attrs.OnMouseOut||function(){attrs.mouse_state="out";UI.Refresh();});
	attrs.OnMouseDown=(attrs.OnMouseDown||function(){attrs.mouse_state="down";UI.Refresh();});
	attrs.OnMouseUp=(attrs.OnMouseUp||function(){attrs.mouse_state="over";UI.Refresh();});
	//////////////////
	//rendering
	UI.RoundRect(attrs);
	var x=attrs.x+padding;
	if(bmpid)UI.DrawBitmap(bmpid,x,attrs.y+(attrs.h-attrs.h_bmp)*0.5,attrs.w_bmp,attrs.h_bmp,attrs.icon_color||0xffffffff);
	x+=attrs.w_bmp;
	UI.DrawTextControl(attrs,x,attrs.y+(attrs.h-attrs.h_text)*0.5,attrs.text_color||0xffffffff)
	return W.Region(id,attrs);
}

W.Edit=function(id,attrs0){
	var attrs=UI.Keep(id,attrs0,1);
	UI.StdStyling(id,attrs,attrs0, "edit",attrs.focus_state||"blur");
	UI.StdAnchoring(id,attrs);
	var ed=attrs.ed;
	if(!ed){
		ed=Duktape.__ui_new_editor(attrs);
		if(attrs.text){ed.MassEdit([0,0,code_text]);}
		attrs.sel0=ed.CreateLocator(0,-1);
		attrs.sel1=ed.CreateLocator(0,1);
		attrs.ed=ed;
		attrs.OnTextEdit=function(event){
			//todo: draw an overlay
		}
		attrs.OnTextInput=function(event){
			var ccnt0=attrs.sel0.ccnt;
			var ccnt1=attrs.sel1.ccnt;
			if(ccnt0>ccnt1){var tmp=ccnt1;ccnt1=ccnt0;ccnt0=tmp;}
			ed.MassEdit([ccnt0,ccnt1-ccnt0,event.text])
			var lg=Duktape.__byte_length(event.text);
			attrs.sel0.ccnt=ccnt0+lg;
			attrs.sel1.ccnt=ccnt0+lg;
			UI.Refresh()
		};
	}
	//todo: scaling - the editor should use unscaled units
	//todo: scrolling
	ed.Render({x:0,y:0,w:attrs.w,h:attrs.h, scr_x:attrs.x,scr_y:attrs.y, scale:(attrs.scale||1)});
	if(UI.HasFocus(attrs)){
		var ed_caret=ed.XYFromCcnt(attrs.sel1.ccnt);
		UI.SetCaret(UI.context_window,attrs.x+ed_caret.x,attrs.y+ed_caret.y,attrs.caret_width||2,UI.GetFontHeight(attrs.font),attrs.caret_color||0xff000000,attrs.caret_flicker||500);
	}
	return attrs;
}
