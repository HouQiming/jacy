#include <string.h>
#include "wrapper_defines.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include <windows.h>

EXPORT HWND SDL_GetHWND(SDL_Window* window){
	SDL_SysWMinfo info;
	memset(&info,0,sizeof(info));
	SDL_VERSION(&info.version);
	if(SDL_GetWindowWMInfo(window,&info)){
		return info.info.win.window;
	}
	return NULL;
}
