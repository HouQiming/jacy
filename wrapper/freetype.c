//qpad: paths_include="..\\osslib\\include"
/*
<package name="Freetype">
	<target n="win32">
		<dll n="objs/release_st/freetype.dll"/>
		<lib n="objs/release_st/freetype.lib"/>
	</target>
	<target n="win64">
		<dll n="builds/windows/vc2008/x64/Release Singlethreaded/freetype64.dll"/>
		<lib n="builds/windows/vc2008/x64/Release Singlethreaded/freetype64.lib"/>
	</target>
	<target n="android">
		<array a="android_so" n="ndk-libs/freetype/libs"/>
		<array a="android_libname" n="freetype"/>
	</target>
	<target n="ios">
		<lib n="mac/pmenv/freetype-2.5.3/libfreetype.a"/>
	</target>
</package>
*/
#include "wrapper_defines.h"
#include "ft2build.h"
#include "freetype.h"
#include "ftglyph.h"
#include "ftsnames.h"

typedef unsigned char u8;
typedef size_t uptr;

static FT_Library g_lib=NULL;

EXPORT FT_Face FTW_LoadFont(char* a,int sz){
	FT_Face ret=NULL;
	if(!g_lib){
		FT_Init_FreeType(&g_lib);
		if(!g_lib)return NULL;
	}
	if(sz){
		FT_New_Memory_Face(g_lib,a,sz,0,&ret);
	}else{
		FT_New_Face(g_lib,a,0,&ret);
    }
    if(ret&&!FT_IS_SCALABLE(ret)){
    	FT_Done_Face(ret);
    	ret=NULL;
    }
    return ret;
}

EXPORT int FTW_SetPixelSize(FT_Face ft,int h,int* metrics){
	/*returns the actual line height*/
	FT_Size_RequestRec req;
	FT_Error err;
	int lineskip;
	int pixel_ascent=0;
	if(FT_IS_SCALABLE(ft)&&(h&0x40000000)){
		//request absolute advance
		int h2,iter;
		h&=0x3fffffff;
		memset(&req,0,sizeof(req));
		req.type=FT_SIZE_REQUEST_TYPE_REAL_DIM;
		req.height=h<<6;
		err=FT_Request_Size(ft,&req);
		for(iter=0;iter<2;iter++){
			if(err)return 0;
			lineskip=FT_MulFix(ft->height,ft->size->metrics.y_scale);
			//lineskip=FT_MulFix((ft->ascender-ft->descender)*9/8,ft->size->metrics.y_scale);
			if(((lineskip+63)>>6)==h)break;
			h2=FT_DivFix(FT_MulFix(h<<6,req.height),lineskip);
			if(h2<128){h2=128;}
			memset(&req,0,sizeof(req));
			req.type=FT_SIZE_REQUEST_TYPE_REAL_DIM;
			req.height=h2;
			err=FT_Request_Size(ft,&req);
		}
	}else{
		h&=0x3fffffff;
		err=FT_Set_Pixel_Sizes(ft,0,h);
	}
	if(err)return 0;
	if(FT_IS_SCALABLE(ft)){
		pixel_ascent=FT_MulFix(ft->ascender,ft->size->metrics.y_scale);
		pixel_ascent=((pixel_ascent+63)>>6);
		lineskip=FT_MulFix(ft->height,ft->size->metrics.y_scale);
		//lineskip=FT_MulFix((ft->ascender-ft->descender)*9/8,ft->size->metrics.y_scale);
		lineskip=((lineskip+63)>>6);
	}else{
		pixel_ascent=h;
		lineskip=h+1;
	}
	metrics[0]=lineskip;
	metrics[1]=pixel_ascent;
	return lineskip;
}

EXPORT u8* FTW_GetCharacter(FT_Face ft,int embolden,int ch,int* ret){
	FT_Error err;
	FT_Long scale;
	int pixel_ascent=0;
	FT_GlyphSlot slot=ft->glyph;
	err=FT_Load_Char(ft,ch&0xffffff,ch<0?FT_LOAD_NO_HINTING:FT_LOAD_DEFAULT);
	if(err)return NULL;
	if(embolden){
		if(slot->format==FT_GLYPH_FORMAT_OUTLINE){
			FT_Outline_Embolden(&slot->outline,embolden);
		}
	}
	FT_Render_Glyph(slot,FT_RENDER_MODE_NORMAL);
	if(FT_IS_SCALABLE(ft)){
		scale=ft->size->metrics.y_scale;
		pixel_ascent=FT_MulFix(ft->ascender, scale);
		pixel_ascent=((pixel_ascent+63)>>6);
	}
	/*return the bitmap dimensions and location -- x,y,w,h, pitch,dx*/
	ret[0]=slot->bitmap_left;
	ret[1]=pixel_ascent-slot->bitmap_top;
	ret[2]=slot->bitmap.width;
	ret[3]=slot->bitmap.rows;
	ret[4]=slot->bitmap.pitch;
	//ret[5]=(slot->advance.x+31)>>6;
	ret[5]=(slot->advance.x+63)>>6;
	//printf("%08x\n",err);
    return slot->bitmap.buffer;
}

EXPORT int FTW_UnloadFont(FT_Face ft){
	return !FT_Done_Face(ft);
}

EXPORT int FTW_GetGlyphId(FT_Face ft,int ch){
	return FT_Get_Char_Index(ft,ch);
}

/*
FT_EXPORT( FT_UInt ) FT_Get_Sfnt_Name_Count( FT_Face  face );
FT_EXPORT( FT_Error ) FT_Get_Sfnt_Name( FT_Face       face,
                FT_UInt       idx,
                FT_SfntName  *aname );
EXPORT char* FTW_GetName(FT_Face ft,int* plg){
	FT_UInt n=FT_Get_Sfnt_Name_Count(ft);
	FT_UInt i;
	FT_SfntName na;
	for(i=0;i<n;i++){
		FT_Error err=FT_Get_Sfnt_Name(ft,idx,&na);
		if(err)continue;
		if(na.name_id)
			na.string;
			na.string_len;
		}
	}
	return NULL;
}
*/
