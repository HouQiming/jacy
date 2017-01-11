#ifndef __SDL_CALLBACKS_H
#define __SDL_CALLBACKS_H
void sdlcbInit();
int sdlcbCheck(void* pevent);
void sdlcbQueue(void* ptr_fn,void* ptr_this,void* param);
void sdlcbQueueSync(void* ptr_fn,void* ptr_this,void* param);
#endif
