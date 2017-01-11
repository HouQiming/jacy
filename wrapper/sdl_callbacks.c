#include <string.h>
#include "wrapper_defines.h"
#include "SDL.h"
#define CALLBACK_EVENT_ID 0x77777777

typedef void(*fjccallback)(void*,void*);
static SDL_TLSID g_sync_tls;
static int g_inited=0;

void sdlcbInit(){
	if(g_inited){return;}
	g_sync_tls=SDL_TLSCreate();
	g_inited=1;
}

int sdlcbCheck(SDL_Event* pevent){
	if(pevent->type==SDL_USEREVENT&&pevent->user.code==CALLBACK_EVENT_ID){
		SDL_sem* sem=(SDL_sem*)(&pevent->user.data2)[2];
		((fjccallback)pevent->user.data1)(pevent->user.data2,(&pevent->user.data2)[1]);
		if(sem){
			SDL_SemPost(sem);
		}
		return 1;
	}else{
		return 0;
	}
}

void sdlcbQueue(void* ptr_fn,void* ptr_this, void* param){
	SDL_Event a;
	memset(&a,0,sizeof(a));
	a.type=SDL_USEREVENT;
	a.user.code=CALLBACK_EVENT_ID;
	a.user.data1=ptr_fn;
	a.user.data2=ptr_this;
	(&a.user.data2)[1]=param;
	(&a.user.data2)[2]=NULL;
	SDL_PushEvent(&a);
}

static void sem_free_cb(void* p){
	if(p){
		SDL_DestroySemaphore((SDL_sem*)p);
	}
}

void sdlcbQueueSync(void* ptr_fn,void* ptr_this, void* param){
	SDL_Event a;
	SDL_sem* sem=(SDL_sem*)SDL_TLSGet(g_sync_tls);
	memset(&a,0,sizeof(a));
	if(!sem){
		sem=SDL_CreateSemaphore(0);
		SDL_TLSSet(g_sync_tls,(void*)sem,sem_free_cb);
	}
	a.type=SDL_USEREVENT;
	a.user.code=CALLBACK_EVENT_ID;
	a.user.data1=ptr_fn;
	a.user.data2=ptr_this;
	(&a.user.data2)[1]=param;
	(&a.user.data2)[2]=(void*)sem;
	SDL_PushEvent(&a);
	SDL_SemWait(sem);
}
