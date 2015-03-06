#include <string.h>
#include "wrapper_defines.h"
#include "SDL.h"
#define TIMER_EVENT_ID 2

static Uint32 SDLCALL timerproc(Uint32 interval, void *param){
	SDL_Event a;
	memset(&a,0,sizeof(a));
	a.type=SDL_USEREVENT;
	a.user.code=TIMER_EVENT_ID;
	a.user.data1=param;
	SDL_PushEvent(&a);
	return interval;
}

EXPORT SDL_TimerID SDL_SetInterval(int interval,void* param){
	return SDL_AddTimer(interval, timerproc, param);
}

static int SDLCALL EventFilterRemoveDupTimers(void* userdata, SDL_Event* pevent){
	if(pevent->type==SDL_USEREVENT&&pevent->user.code==TIMER_EVENT_ID&&pevent->user.data1==userdata){
		//a duplicate timer
		return 0;
	}else{
		return 1;
	}
}

EXPORT void SDL_ClearInterval(SDL_TimerID timer_id,void* userdata){
	SDL_RemoveTimer(timer_id);
	SDL_FilterEvents(EventFilterRemoveDupTimers,userdata);
}
