/*
void face_init(char* fname_violajones,char* fname_dde);
int face_run(unsigned char* img_raw,int w,int h);
float* face_get(char* name,int* pdim);

<package name="Blackbox_Face_Dll">
	<target n="win32">
	</target>
	<target n="win64">
	</target>
</package>
*/
#include <stdio.h>
#include <malloc.h>
#include "../osslib/commercial/ddeface/ddeface.h"
#include "../units/wrapper_defines.h"
typedef enum{
	STATE_NONE=0,
	STATE_GOT_FACERECT,
	STATE_GOT_DDE,
}STATE_ENUM;

//one-face special case
static TWorkArea* g_dde_context=NULL;
static float* g_global_tables=NULL;
static STATE_ENUM g_state=STATE_NONE;
extern void osal_InitFaceDetector(char* xml_name);
extern int __cdecl osal_DetectFaces(unsigned char* img_raw,int w,int h, int* retbuf,int max_nfaces);

EXPORT void face_init(char* fname_violajones,char* fname_dde){
	FILE* f;
	int sz;
	osal_InitFaceDetector(fname_violajones);
	f=fopen(fname_dde,"rb");
	fseek(f,0,SEEK_END);
	sz=ftell(f);
	fseek(f,0,SEEK_SET);
	fread(g_global_tables=(float*)malloc(sz),1,sz,f);
	fclose(f);
	init_global_tables(g_global_tables);
	g_dde_context=(TWorkArea*)malloc(dde_context_size());
}

static int g_face_bb[4];
static float g_face_bbf[4];
EXPORT void face_reset(){
	g_state=STATE_NONE;
}

EXPORT int face_run(unsigned char* img_raw,int w,int h){
	if(g_state==STATE_NONE){
		int n_faces=osal_DetectFaces(img_raw,w,h, g_face_bb,1);
		if(n_faces){g_state=STATE_GOT_FACERECT;}
		//printf("n_faces %d\n",n_faces);
	}
	if(g_state==STATE_GOT_FACERECT){
		int is_valid;
		int i;
		for(i=0;i<4;i++){
			g_face_bbf[i]=(float)g_face_bb[i];
		}
		is_valid=hldde_first(g_dde_context, img_raw,w*4,w,h, g_face_bbf);
		//printf("hldde_first %d\n",is_valid);
		if(is_valid){g_state=STATE_GOT_DDE;}else{g_state=STATE_NONE;}
	}else if(g_state==STATE_GOT_DDE){
		int is_valid=hldde_next(g_dde_context, img_raw,w*4,w,h);
		if(is_valid){g_state=STATE_GOT_DDE;}else{g_state=STATE_NONE;}
	}
	return g_state==STATE_GOT_DDE;
}

EXPORT float* face_get(char* name,int* pdim){
	return dde_get(g_dde_context,name,pdim);
}
