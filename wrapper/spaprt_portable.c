 /*
<package name="SDL">
	<target n="win32">
		<dll n="lib/x86/sdl2.dll"/>
		<lib n="lib/x86/sdl2.lib"/>
	</target>
	<target n="win64">
		<dll n="lib/x64/sdl2.dll"/>
		<lib n="lib/x64/sdl2.lib"/>
	</target>
	<target n="android">
	</target>
	<target n="ios">
	</target>
	<target n="mac">
		<array a="mac_frameworks" n="System/Library/Frameworks/AudioToolbox.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/AudioUnit.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/Cocoa.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/CoreAudio.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/IOKit.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/OpenGL.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/ForceFeedback.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/CoreFoundation.framework"/>
		<array a="mac_frameworks" n="System/Library/Frameworks/Carbon.framework"/>
	</target>
	<target n="linux">
	</target>
	<var n="use_spaprt_portable" v="1"/>
</package>

In the android mode, the sdl stuff are set by the build script. We don't have to worry about that here.
*/
//the sjlj approach really doesn't work if we share stack across the worker and the fiber?
//but if we don't get too ambitious... no, this is doomed anyway, gotta emulate using makecontext
//how about real threading - we're not perf critical on fibers, the async call isn't cheap either
//for efficiency, greenthread is faster
//#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#ifdef LINUX
#undef _WIN32
#define NEED_MAIN_WRAPPING
#endif
#ifndef _WIN32
#include <sys/mman.h>
#include <sys/types.h>
///////////
#include <sys/param.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <wchar.h>
#include <sys/wait.h>
///////////
#ifndef PM_RELEASE
#include <signal.h>
#endif
#endif
#if defined(ANDROID)||defined(__ANDROID__)
#include <android/log.h>
#endif
//#define FORCE_SDL

#if defined(_WIN32)&&!defined(FORCE_SDL)
////////////////////////Win32
#include <windows.h>
typedef INT_PTR iptr;
#define THREADCALL WINAPI
#define THREADRET DWORD
#define CloseEvent CloseHandle
#define CloseThread CloseHandle
typedef long long i64;
static i64 GetPerformanceCounter(){
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}
static i64 GetPerformanceFrequency(){
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	return li.QuadPart;
}

typedef HANDLE HEVENT;
typedef HANDLE HMUTEX;
typedef HANDLE HTHREAD;
typedef DWORD TLSID;
#define TryBlockOn(handle) (WaitForSingleObject(handle,0)==WAIT_OBJECT_0)
#define BlockOn(handle) WaitForSingleObject(handle,INFINITE)
#define BlockOnEvent(handle) WaitForSingleObject(handle,INFINITE)
#else

////////////////////////SDL
#if defined(ANDROID)||defined(__ANDROID__)
	#include "SDL.h"
	#define __declspec(arg)
#elif defined(__APPLE__)
	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		#include "SDL/include/SDL.h"
	#elif TARGET_OS_IPHONE
		// iOS device
		#include "SDL/include/SDL.h"
	#elif TARGET_OS_MAC
		// Mac OS
		#include "SDL/include/SDL.h"
		#define HAS_GLOB
		#define NEED_MAIN_WRAPPING
		#include <glob.h>
	#else
		// Unsupported platform
	#endif
	#define __declspec(arg)
#else
	#include "SDL.h"
	#ifndef _WIN32
		#define __declspec(arg)
	#endif
#endif

#ifdef LINUX
//emulate the windows-based custom extensions
char* SDL_GetInputEventText(SDL_Event* pevent){return pevent->text.text;}
void SDL_FreeInputEventText(SDL_Event* pevent){}

#endif

#define __cdecl
typedef intptr_t iptr;
/*
#ifdef _M_AMD64
typedef long long iptr;
#else
typedef long iptr;
#endif
*/

#define malloc SDL_malloc
#define calloc SDL_calloc
#define free SDL_free
#define THREADCALL SDLCALL
#define THREADRET int
#define CreateThread(secure,stack, fn, param, flags,ptid) SDL_CreateThread(fn,"f",param)
#define GetPerformanceCounter SDL_GetPerformanceCounter
#define GetPerformanceFrequency SDL_GetPerformanceFrequency
#define CloseThread SDL_DetachThread
typedef Sint64 i64;
typedef SDL_sem* HEVENT;
typedef SDL_mutex* HMUTEX;
typedef SDL_Thread* HTHREAD;

typedef SDL_TLSID TLSID;
#define TlsAlloc SDL_TLSCreate
#define TlsGetValue SDL_TLSGet
#define TlsSetValue(a,b) SDL_TLSSet(a,b,NULL)
#define CreateEventA(p0,p1,p2,p3) SDL_CreateSemaphore(0)
#define CreateMutexA(p0,p1,p2) SDL_CreateMutex()
#define SetEvent(handle) SDL_SemPost(handle)
#define TryBlockOn(handle) (SDL_TryLockMutex(handle)==0)
#define BlockOnEvent(handle) SDL_SemWait(handle)
#define BlockOn(handle) SDL_LockMutex(handle)
#define ReleaseMutex(handle) SDL_UnlockMutex(handle)
#define CloseEvent(handle) SDL_DestroySemaphore(handle)
#define SwitchToThread() SDL_Delay(0)
#define Sleep(a) SDL_Delay(a)

static int IsBadReadPtr(void* p,size_t sz){return !p;}
#endif

#ifdef PM_RELEASE
#define EXPORT
#else
#define EXPORT __declspec(dllexport)
#endif

/////////////////////////////////////////////////////////////////
//the coroutine system
/////////////////////////////////////////////////////////////////
//thread - sjlj fiber, the thread could itself serve as the work thread
//could have internally "switch_to_thread" and "switch_to_fiber"
//Sleep could be an ordinary wait
//pool the threads
//exposing risks correctness problems -- e.g. rc
#define TLJ_START_POINT_WAIT_FOR_SWITCH 1
#define TLJ_START_POINT_DISCARD 2
#define MSG_UNBLOCKED 0
#define MSG_TERMINATED 1
#define MSG_DISCARD 2
#define CO_SUSPENDED (1<<30)
#define CO_FROM_SPAP 0x20000000
typedef void(__cdecl*PF_COROUTINE)(void*);
typedef int(__cdecl*PF_COM_RELEASE)(void*);
typedef struct _TCoroutine{
	struct _TCoroutine* next;
	///////////
	HEVENT event_launch;
	PF_COROUTINE f;
	void* param;
	int flags,message;
	i64 t_last_executed;
}TCoroutine;

static HMUTEX g_coroutine_lock;
//static HMUTEX g_coroutine_lock2;

static int g_inited=0;
static TLSID g_tls_pcoroutine;
static i64 g_freq=(i64)1;

static TCoroutine* g_free_coroutines=NULL;
static volatile int g_yielding_atomic=0;

EXPORT void starcClassFree(void* p);

THREADRET THREADCALL tlj_starter(void* pcoroutine_v){
	TCoroutine* pcoroutine=(TCoroutine*)pcoroutine_v;
	//int mode;
	//set up the dispatcher so that we can reuse the thread
	pcoroutine->event_launch=CreateEventA(NULL,0,0,NULL);
	TlsSetValue(g_tls_pcoroutine, pcoroutine_v);
	for(;;){
		BlockOn(g_coroutine_lock);
		pcoroutine->f(pcoroutine->param);
		if((pcoroutine->flags&CO_FROM_SPAP)&&pcoroutine->param){
			//do gcRelease of the function
			if(!--((iptr*)pcoroutine->param)[-1]){
				starcClassFree(pcoroutine->param);
			}
		}
		pcoroutine->next=g_free_coroutines;
		g_free_coroutines=pcoroutine;
		ReleaseMutex(g_coroutine_lock);
		//wait for the scheduler to get us out of the pool, reuse the switching event
		BlockOnEvent(pcoroutine->event_launch);
		if(pcoroutine->message==MSG_DISCARD){
			free(pcoroutine);
			break;
		}
	}
	return 0;
}

////////////////////////////////////////////////
//emulate the old interface
EXPORT void spapInitCoroutines(){
	if(!g_inited){
		g_inited=1;
		g_tls_pcoroutine=TlsAlloc();
		g_freq=GetPerformanceFrequency();
		g_coroutine_lock=CreateMutexA(NULL,0,NULL);
		//g_coroutine_lock2=CreateMutexA(NULL,0,NULL);
		BlockOn(g_coroutine_lock);
		//BlockOn(g_coroutine_lock2);
	}
}

EXPORT TCoroutine* spapCreateCoroutine(PF_COROUTINE f,void* param,int flags){
	#ifdef _WIN32
		int tid=0;
	#endif
	HTHREAD hthread;
	TCoroutine* ret;
	if(g_free_coroutines){
		ret=g_free_coroutines;
		g_free_coroutines=ret->next;
		ret->next=NULL;
		ret->f=f;
		ret->param=param;
		ret->flags=flags;
		SetEvent(ret->event_launch);
	}else{
		ret=malloc(sizeof(TCoroutine));
		memset(ret,0,sizeof(TCoroutine));
		ret->f=f;
		ret->param=param;
		ret->flags=flags;
		hthread=CreateThread(NULL,0, tlj_starter, ret, 0,&tid);
		if(!hthread){
			free(ret);
			return NULL;
		}
		CloseThread(hthread);
	}
	return ret;
}

EXPORT void spapRunBlockingFunction(PF_COROUTINE f,void* param){
	//printf("g_yielding_atomic=%d\n",g_yielding_atomic);
	if(g_yielding_atomic){
		f(param);
		return;
	}
	ReleaseMutex(g_coroutine_lock);
	f(param);
	//BlockOn(g_coroutine_lock2);ReleaseMutex(g_coroutine_lock2);
	BlockOn(g_coroutine_lock);
}

EXPORT void spapBlockYielding(){
	g_yielding_atomic++;
}

EXPORT void spapUnblockYielding(){
	g_yielding_atomic--;
}

EXPORT void spapStartRunningCoroutines(){
	ReleaseMutex(g_coroutine_lock);
	//ReleaseMutex(g_coroutine_lock2);
}

EXPORT void spapStopRunningCoroutines(){
	//BlockOn(g_coroutine_lock2);
	BlockOn(g_coroutine_lock);
}

//event_wake_up
EXPORT void spapCoroutineSleep(int duration_ms){
	ReleaseMutex(g_coroutine_lock);
	if(!duration_ms){
		SwitchToThread();
	}else{
		Sleep(duration_ms);
	}
	//BlockOn(g_coroutine_lock2);ReleaseMutex(g_coroutine_lock2);
	BlockOn(g_coroutine_lock);
}

EXPORT void spapSetCoroutineFlag(TCoroutine* pcoroutine,int flags){
	pcoroutine->flags=flags;
}
EXPORT int spapGetCoroutineFlag(TCoroutine* pcoroutine){
	return pcoroutine->flags;
}

EXPORT void spapFreeCoroutineResources(){
	TCoroutine* pcoroutine=NULL;
	TCoroutine* pdiscard_head=g_free_coroutines;
	int n=0,i=0;
	TCoroutine** ppcoroutines=NULL;
	g_free_coroutines=NULL;
	for(pcoroutine=pdiscard_head;pcoroutine;pcoroutine=pcoroutine->next){
		pcoroutine->message=MSG_DISCARD;
		n++;
	}
	ppcoroutines=(TCoroutine**)malloc(sizeof(TCoroutine*)*n);
	for(pcoroutine=pdiscard_head;pcoroutine;pcoroutine=pcoroutine->next){
		ppcoroutines[i]=pcoroutine;
		i++;
	}
	for(i=0;i<n;i++){
		//after the SetEvent, the TCoroutine can become invalid immediately, thus we need the temporary array
		SetEvent(ppcoroutines[i]->event_launch);
	}
	free(ppcoroutines);
}

EXPORT TCoroutine* spapGetCurrentCoroutine(){
	TCoroutine* ret=TlsGetValue(g_tls_pcoroutine);
	return ret;
}

///////////////////////////////////////////////
typedef unsigned int u32;
typedef struct _TClassDesc{
	iptr sz,alg;
	PF_COROUTINE dtor;
	char* name;
	u32 hash[2];
	union{
		iptr _nrcitem;
		int nrcitem;
	};
}TClassDesc;

//dozero should always be 2
EXPORT void* starcClassAlloc(int dozero,TClassDesc* cd){
	iptr size=cd->sz;
	iptr szmisc=sizeof(void*)*2;
	iptr pbase;
	void* ret;
	//our malloc is 16-aligned by default
	//puts(cd->name);
	if(cd->alg>sizeof(void*)){szmisc+=sizeof(void*);}
	pbase=(iptr)malloc(size+szmisc+(cd->alg>sizeof(void*)?cd->alg:0));
	if(!pbase)return NULL;
	assert(dozero==2);
	if(cd->alg>sizeof(void*)){
		//printf("%d\n",cd->alg);
		ret=(void*)((pbase+szmisc+cd->alg-1)&-cd->alg);
		((void**)ret)[-3]=(void*)pbase;
	}else{
		ret=(void*)(pbase+szmisc);
	}
	((void**)ret)[-2]=(void*)cd;
	((void**)ret)[-1]=(void*)(iptr)1;
	memset(ret,0,size);
	return ret;
}

//BSGP? mobile CUDA... we still need it
void _bsgp_delete(void* selfv);
typedef struct _TBsgpObject{
	void* __vftab;
	iptr __refcnt;
	PF_COROUTINE __free;
}TBsgpObject;
void bsgp_classRelease(void* pv){
	if(pv){
		iptr* p=(iptr*)pv;
		iptr a=p[1]-1;
		p[1]=a;
		if(!a){_bsgp_delete(p);}
	}
}
void _bsgp_delete(void* selfv){
	TBsgpObject* self=(TBsgpObject*)selfv;
	iptr* pf=(iptr*)(self->__vftab);
	iptr f;
	iptr i,n;
	pf+=pf[-1];
	f=pf[0];
	if(f){
		self->__refcnt++;
		((PF_COROUTINE)f)(self);
		if(--self->__refcnt)return;
	}
	n=pf[1];
	pf+=2;
	for(i=0;i<n;i++){
		bsgp_classRelease(*(void**)((iptr)(self)+pf[i]));
	}
	self->__free(self);
}
EXPORT void starc_classRelease(void* pv){
	if(pv){
		iptr* p=(iptr*)pv;
		iptr a=p[-1]-1;
		p[-1]=a;
		if(!a){starcClassFree(p);}
	}
}
EXPORT void starcClassFree(void* p){
	TClassDesc* cd=((TClassDesc**)p)[-2];
	if(cd->dtor){
		//we can't have this thing re-released during dtor
		((iptr*)p)[-1]=(iptr)0x80000000;
		cd->dtor(p);
	}
	/////////////////////////
	//free members
	//printf("free %s %d\n",cd->name,cd->nrcitem);
	if(cd->nrcitem>0){
		iptr* prcitems=((iptr*)(cd+1));
		iptr i;
		//writeln(cd.name,' ',cd.nrcitem)
		for(i=0;i<cd->nrcitem;i++){
			iptr d=prcitems[i];
			int mode=((int)d&3);
			void* pp=*(void**)((char*)p+(d&(iptr)(-4)));
			switch(mode){
			break;case 1://SPAP# interface
				if(pp)pp=((void**)pp)[1];
			case 0://SPAP#
				starc_classRelease(pp);
			break;case 2://COM
				if(pp){((PF_COM_RELEASE**)pp)[0][1](pp);}
			break;case 3://BSGP
				bsgp_classRelease(pp);
			}
			//writeln
		}
	}
	/////////////////////////
	if(cd->alg>sizeof(void*)){
		free(((void**)p)[-3]);
	}else{
		free((iptr*)p-2);
	}
}

/////////////////////////////////////////
//portable call stack tracking
typedef struct _TLineInfoItem{
	char* fn;
	unsigned short l0,c0,l1,c1;
}TLineInfoItem;

typedef struct _TCallStackItem{
	struct _TCallStackItem* dad;
	char* fnn;
	TLineInfoItem* li;
}TCallStackItem;

typedef struct _TLineInfoItemEx{
	const char* s;
	const struct _TLineInfoItemEx* next;
}TLineInfoItemEx;

typedef struct _TCallStackItemEx{
	struct _TCallStackItemEx* dad;
	const char* fnn;
	const TLineInfoItemEx* li;
}TCallStackItemEx;

#ifdef PM_RELEASE
	EXPORT void osal_WriteLog(char* buf){
		//nothing
	}
	//and define it out in our own file
	#define osal_WriteLog(s)
#else
	#if defined(ANDROID)||defined(__ANDROID__)
		EXPORT void osal_WriteLog(char* buf){
			__android_log_write(ANDROID_LOG_ERROR, "STDOUT",buf);
		}
	#else
	#ifdef _WIN32
		EXPORT void osal_WriteLog(char* buf){
			HANDLE herror=GetStdHandle(STD_ERROR_HANDLE);
			DWORD dummy=0;
			WriteFile(herror,buf,strlen(buf),&dummy,NULL);
			//FILE* f=fopen("CONOUT$","wb");
			//fprintf(f,"%s",buf);
			//fflush(f);
			//fclose(f);
		}
	#else
		EXPORT void osal_WriteLog(char* buf){
			fprintf(stderr,"%s",buf);
		}
	#endif
	#endif
#endif

#ifndef PM_RELEASE
	void dump_call_stack(char*);
#endif
EXPORT void spapReportErrorf(char* fmt,...){
	//the formatting isn't important
	va_list v;
	#ifndef PM_RELEASE
		va_start(v,fmt);
		#if defined(ANDROID)||defined(__ANDROID__)
			__android_log_vprint(ANDROID_LOG_ERROR,"STDOUT",fmt,v);
		#else
			vfprintf(stderr,fmt,v);
			fprintf(stderr,"\n");
		#endif
		dump_call_stack("runtime error\n");
		exit(1);
	#endif
}
EXPORT void spapReportError(char* s){
	#ifndef PM_RELEASE
		osal_WriteLog(s);
		osal_WriteLog("\n");
		dump_call_stack("runtime error\n");
		exit(1);
	#endif
}

#ifdef PM_RELEASE
EXPORT void spapAddCallStack(TCallStackItem* p){}
EXPORT void* spapGetCallStack(){return NULL;}
EXPORT void spapSetCallStack(void* p){}
/////////////////
//debugging system v2
EXPORT void spapDebugV2Start(){}
EXPORT void spapPushCallStack(TCallStackItemEx* p){}
EXPORT void spapPopCallStack(){}
#else
static int g_tls_cur_stack=0;
static int g_debug_inited=0;//version number after init
static HMUTEX g_stackdump_lock;
static char dump_temp[1024];

void dump_call_stack(char* reason){
	TCallStackItem* stk;
	BlockOn(g_stackdump_lock);
	osal_WriteLog(reason);
	for(stk=(TCallStackItem*)TlsGetValue(g_tls_cur_stack);stk;stk=stk->dad){
		//__android_log_print(ANDROID_LOG_ERROR,"STDOUT",">>> stk=%p",stk);
		if(IsBadReadPtr(stk,sizeof(*stk)))break;
		if(g_debug_inited==2){
			const TLineInfoItemEx* li=(const TLineInfoItemEx*)stk->li;
			int p_dump=0,lg_fn=0;
			for(;li;li=li->next){
				const char* s=li->s;
				int lg=strlen(s);
				if(p_dump+lg>=1024){
					dump_temp[p_dump]=0;
					osal_WriteLog(dump_temp);
					p_dump=0;
				}
				memcpy(dump_temp+p_dump,s,lg);
				p_dump+=lg;
				if(dump_temp[p_dump-1]=='\n'){
					dump_temp[p_dump]=0;
					osal_WriteLog(dump_temp);
					p_dump=0;
				}
			}
			lg_fn=strlen(stk->fnn);
			memcpy(dump_temp+p_dump,stk->fnn,lg_fn);
			p_dump+=lg_fn;
			dump_temp[p_dump]=0;
			osal_WriteLog(dump_temp);
		}else{
			TLineInfoItem* li=stk->li;
			//__android_log_print(ANDROID_LOG_ERROR,"STDOUT",">>> li=%p",li);
			//__android_log_print(ANDROID_LOG_ERROR,"STDOUT",">>> stk->fnn=%s",stk->fnn);
			if(!IsBadReadPtr(li,sizeof(*li))&&li->fn[0]!='<'){
				int l0=(int)li->l0;
				int c0=(int)li->c0;
				int l1=(int)li->l1;
				int c1=(int)li->c1;
				dump_temp[0]=0;
				sprintf(dump_temp,"\"%s\"",li->fn);
				if(l0==l1){
					sprintf(dump_temp+strlen(dump_temp)," %d,%d-%d",l0,c0,c1);
				}else{
					sprintf(dump_temp+strlen(dump_temp)," %d,%d-%d,%d",l0,c0,l1,c1);
				}
				sprintf(dump_temp+strlen(dump_temp),": %s\n",stk->fnn);
				osal_WriteLog(dump_temp);
			}
		}
	}
	ReleaseMutex(g_stackdump_lock);
	#if defined(ANDROID)||defined(__ANDROID__)
		//nothing
	#else
		fflush(stdout);
		fflush(stderr);
	#endif
}

#ifdef _WIN32
static LONG __stdcall exceptionCatcher(EXCEPTION_POINTERS *pep);
static LPTOP_LEVEL_EXCEPTION_FILTER std_eh=NULL;
static void* hvexceptionCatcher=NULL;
static LONG __stdcall exceptionCatcher(EXCEPTION_POINTERS *pep){
	char* reason="CRASHED!\n";
	if(hvexceptionCatcher){
		if(pep->ExceptionRecord->ExceptionCode==0x6ba){
			//shell exception
			return EXCEPTION_CONTINUE_SEARCH;
		}
		RemoveVectoredExceptionHandler(hvexceptionCatcher);
	}
	SetUnhandledExceptionFilter(std_eh);
	switch(pep->ExceptionRecord->ExceptionCode){
	case EXCEPTION_ACCESS_VIOLATION:
		reason="access violation\n";
		break;
	case EXCEPTION_BREAKPOINT:
		reason="break point\n";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		reason="data misalignment\n";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		reason="divide by zero\n";
		break;
	case EXCEPTION_INT_OVERFLOW:
		reason="integer overflow\n";
		break;
	case EXCEPTION_STACK_OVERFLOW:
		reason="stack overflow\n";
		break;
	}
	dump_call_stack(reason);
	return EXCEPTION_CONTINUE_SEARCH;
}
#else
static struct sigaction g_old_action;
static struct sigaction g_old_action2;
static struct sigaction g_old_action3;
void unix_handler(int sigid){
	char* reason="CRASHED!\n";
	switch(sigid){
	case SIGSEGV:
		reason="access violation\n";
		break;
	case SIGABRT:
		reason="aborted\n";
		break;
	case SIGBUS:
		reason="data misalignment\n";
		break;
	}
	dump_call_stack(reason);
	sigaction(SIGSEGV,&g_old_action,NULL);
	sigaction(SIGABRT,&g_old_action2,NULL);
	sigaction(SIGBUS,&g_old_action3,NULL);
	raise(sigid);
}
#endif

static void initDebuggingGlobals(){
	g_tls_cur_stack=TlsAlloc();
	g_stackdump_lock=CreateMutexA(NULL,0,NULL);
	TlsSetValue(g_tls_cur_stack,NULL);
	#ifdef _WIN32
		{
			char scrap[260];
			std_eh=SetUnhandledExceptionFilter(exceptionCatcher);
			//this is required for a windows bug...
			scrap[0]=0;
			GetEnvironmentVariableA("__SPAPRT_INTERCEPT_WNDPROC_CRASH",scrap,sizeof(scrap)-1);
			if(scrap[0]=='1'){
				hvexceptionCatcher=AddVectoredExceptionHandler(0,exceptionCatcher);
			}
		}
	#else
		struct sigaction act;
		memset(&act,0,sizeof(act));
		memset(&g_old_action,0,sizeof(g_old_action));
		act.sa_handler=&unix_handler;
		act.sa_flags=0;
		sigaction(SIGSEGV,&act,&g_old_action);
		sigaction(SIGABRT,&act,&g_old_action2);
		sigaction(SIGBUS,&act,&g_old_action3);
		//__android_log_print(ANDROID_LOG_ERROR,"STDOUT",">>> init debug system, returns %d, old flags == %d",ret,g_old_action.sa_flags);
	#endif
}

EXPORT void spapAddCallStack(TCallStackItem* p){
	//__android_log_write(ANDROID_LOG_ERROR,"STDOUT","spapAddCallStack");
	if(p){
		if(!g_debug_inited){
			//__android_log_write(ANDROID_LOG_ERROR,"STDOUT",">>> init debug system");
			g_debug_inited=1;
			initDebuggingGlobals();
		}
		p->dad=(void*)TlsGetValue(g_tls_cur_stack);
		TlsSetValue(g_tls_cur_stack,p);
	}else{
		p=(void*)TlsGetValue(g_tls_cur_stack);
		if(p){
			TlsSetValue(g_tls_cur_stack,p->dad);
		}
	}
}

EXPORT void* spapGetCallStack(){return (void*)TlsGetValue(g_tls_cur_stack);}
	
EXPORT void spapSetCallStack(void* p){TlsSetValue(g_tls_cur_stack,p);}

EXPORT void spapDebugV2Start(){
	if(!g_debug_inited){
		g_debug_inited=2;
		initDebuggingGlobals();
	}
}

EXPORT void spapPushCallStack(TCallStackItemEx* p){
	p->dad=(void*)TlsGetValue(g_tls_cur_stack);
	TlsSetValue(g_tls_cur_stack,p);
}

EXPORT void spapPopCallStack(){
	TCallStackItemEx* p=(TCallStackItemEx*)TlsGetValue(g_tls_cur_stack);
	TlsSetValue(g_tls_cur_stack,p->dad);
}

#endif
/////////////////////////////////////////////////////////////////
//unix wrappers
#if !defined(_WIN32)
#if !defined(HAS_GLOB) 
///////////////////////////////
//glob.c
/*
 * Natanael Arndt, 2011: removed collate.h dependencies
 *	(my changes are trivial)
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.	IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)glob.c	8.3 (Berkeley) 10/13/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
//__FBSDID("$FreeBSD$");

/*
 * glob(3) -- a superset of the one defined in POSIX 1003.2.
 *
 * The [!...] convention to negate a range is supported (SysV, Posix, ksh).
 *
 * Optional extra services, controlled by flags not defined by POSIX:
 *
 * GLOB_QUOTE:
 *	Escaping convention: \ inhibits any special meaning the following
 *	character might have (except \ at end of string is retained).
 * GLOB_MAGCHAR:
 *	Set in gl_flags if pattern contained a globbing character.
 * GLOB_NOMAGIC:
 *	Same as GLOB_NOCHECK, but it will only append pattern if it did
 *	not contain any magic characters.  [Used in csh style globbing]
 * GLOB_ALTDIRFUNC:
 *	Use alternately specified directory access functions.
 * GLOB_TILDE:
 *	expand ~user/foo to the /home/dir/of/user/foo
 * GLOB_BRACE:
 *	expand {1,2}{a,b} to 1a 1b 2a 2b
 * gl_matchc:
 *	Number of matches in the current invocation of glob.
 */

/*
 * Some notes on multibyte character support:
 * 1. Patterns with illegal byte sequences match nothing - even if
 *	  GLOB_NOCHECK is specified.
 * 2. Illegal byte sequences in filenames are handled by treating them as
 *	  single-byte characters with a value of the first byte of the sequence
 *	  cast to wchar_t.
 * 3. State-dependent encodings are not currently supported.
 */

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.	IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)glob.h	8.1 (Berkeley) 6/2/93
 * $FreeBSD$
 */

#ifndef _GLOB_H_
#define	_GLOB_H_

#include <sys/cdefs.h>
#ifdef LINUX
#include <sys/types.h>
#else
#include <sys/_types.h>
#endif

struct stat;
typedef struct {
	size_t gl_pathc;	/* Count of total paths so far. */
	size_t gl_matchc;	/* Count of paths matching pattern. */
	size_t gl_offs;		/* Reserved at beginning of gl_pathv. */
	int gl_flags;		/* Copy of flags parameter to glob. */
	char **gl_pathv;	/* List of paths matching pattern. */
				/* Copy of errfunc parameter to glob. */
	int (*gl_errfunc)(const char *, int);

	/*
	 * Alternate filesystem access methods for glob; replacement
	 * versions of closedir(3), readdir(3), opendir(3), stat(2)
	 * and lstat(2).
	 */
	void (*gl_closedir)(void *);
	struct dirent *(*gl_readdir)(void *);
	void *(*gl_opendir)(const char *);
	int (*gl_lstat)(const char *, struct stat *);
	int (*gl_stat)(const char *, struct stat *);
} glob_t;

/* Believed to have been introduced in 1003.2-1992 */
#define	GLOB_APPEND	0x0001	/* Append to output from previous call. */
#define	GLOB_DOOFFS	0x0002	/* Use gl_offs. */
#define	GLOB_ERR	0x0004	/* Return on error. */
#define	GLOB_MARK	0x0008	/* Append / to matching directories. */
#define	GLOB_NOCHECK	0x0010	/* Return pattern itself if nothing matches. */
#define	GLOB_NOSORT	0x0020	/* Don't sort. */
#define	GLOB_NOESCAPE	0x2000	/* Disable backslash escaping. */

/* Error values returned by glob(3) */
#define	GLOB_NOSPACE	(-1)	/* Malloc call failed. */
#define	GLOB_ABORTED	(-2)	/* Unignored error. */
#define	GLOB_NOMATCH	(-3)	/* No match and GLOB_NOCHECK was not set. */
#define	GLOB_NOSYS	(-4)	/* Obsolete: source comptability only. */

#define	GLOB_ALTDIRFUNC	0x0040	/* Use alternately specified directory funcs. */
#define	GLOB_BRACE	0x0080	/* Expand braces ala csh. */
#define	GLOB_MAGCHAR	0x0100	/* Pattern had globbing characters. */
#define	GLOB_NOMAGIC	0x0200	/* GLOB_NOCHECK without magic chars (csh). */
#define	GLOB_QUOTE	0x0400	/* Quote special chars with \. */
#define	GLOB_TILDE	0x0800	/* Expand tilde names from the passwd file. */
#define	GLOB_LIMIT	0x1000	/* limit number of returned paths */

/* source compatibility, these are the old names */
#define GLOB_MAXPATH	GLOB_LIMIT
#define	GLOB_ABEND	GLOB_ABORTED

__BEGIN_DECLS
int	glob(const char *, int, int (*)(const char *, int), glob_t *);
void	globfree(glob_t *);
__END_DECLS

#endif /* !_GLOB_H_ */

#define	DOLLAR		'$'
#define	DOT		'.'
#define	EOS		'\0'
#define	LBRACKET	'['
#define	NOT		'!'
#define	QUESTION	'?'
#define	QUOTE		'\\'
#define	RANGE		'-'
#define	RBRACKET	']'
#define	SEP		'/'
#define	STAR		'*'
#define	TILDE		'~'
#define	UNDERSCORE	'_'
#define	LBRACE		'{'
#define	RBRACE		'}'
#define	SLASH		'/'
#define	COMMA		','

#undef DEBUG
#ifndef DEBUG

#define	M_QUOTE		0x8000000000ULL
#define	M_PROTECT	0x4000000000ULL
#define	M_MASK		0xffffffffffULL
#define	M_CHAR		0x00ffffffffULL

typedef uint_fast64_t Char;

#else

#define	M_QUOTE		0x80
#define	M_PROTECT	0x40
#define	M_MASK		0xff
#define	M_CHAR		0x7f

typedef char Char;

#endif


#define	CHAR(c)		((Char)((c)&M_CHAR))
#define	META(c)		((Char)((c)|M_QUOTE))
#define	M_ALL		META('*')
#define	M_END		META(']')
#define	M_NOT		META('!')
#define	M_ONE		META('?')
#define	M_RNG		META('-')
#define	M_SET		META('[')
#define	ismeta(c)	(((c)&M_QUOTE) != 0)


static int	 compare(const void *, const void *);
static int	 g_Ctoc(const Char *, char *, size_t);
static int	 g_lstat(Char *, struct stat *, glob_t *);
static DIR	*g_opendir(Char *, glob_t *);
static const Char *g_strchr(const Char *, wchar_t);
#ifdef notdef
static Char	*g_strcat(Char *, const Char *);
#endif
static int	 g_stat(Char *, struct stat *, glob_t *);
static int	 glob0(const Char *, glob_t *, size_t *);
static int	 glob1(Char *, glob_t *, size_t *);
static int	 glob2(Char *, Char *, Char *, Char *, glob_t *, size_t *);
static int	 glob3(Char *, Char *, Char *, Char *, Char *, glob_t *, size_t *);
static int	 globextend(const Char *, glob_t *, size_t *);
static const Char *	
		 globtilde(const Char *, Char *, size_t, glob_t *);
static int	 globexp1(const Char *, glob_t *, size_t *);
static int	 globexp2(const Char *, const Char *, glob_t *, int *, size_t *);
static int	 match(Char *, Char *, Char *);
#ifdef DEBUG
static void	 qprintf(const char *, Char *);
#endif

//the stupid unix Unicode API is screwed up on Android
static int _mbrtowc(wchar_t* pwc,const char*ps,int dum0,mbstate_t* dum1){
	*pwc=(wchar_t)(unsigned char)(*ps);
	return !!(*pwc);
}

static int _wcrtomb(char* pmb,wchar_t wc,mbstate_t* dum1){
	*pmb=(char)wc;
	return 1;
}

#ifndef ARG_MAX
#define ARG_MAX 14500
#endif
int
glob(const char *pattern, int flags, int (*errfunc)(const char *, int), glob_t *pglob)
{
	const char *patnext;
	size_t limit;
	Char *bufnext, *bufend, patbuf[MAXPATHLEN], prot;
	mbstate_t mbs;
	wchar_t wc;
	size_t clen;

	patnext = pattern;
	if (!(flags & GLOB_APPEND)) {
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		if (!(flags & GLOB_DOOFFS))
			pglob->gl_offs = 0;
	}
	if (flags & GLOB_LIMIT) {
		limit = pglob->gl_matchc;
		if (limit == 0)
			limit = ARG_MAX;
	} else
		limit = 0;
	pglob->gl_flags = flags & ~GLOB_MAGCHAR;
	pglob->gl_errfunc = errfunc;
	pglob->gl_matchc = 0;

	bufnext = patbuf;
	bufend = bufnext + MAXPATHLEN - 1;
	if (flags & GLOB_NOESCAPE) {
		memset(&mbs, 0, sizeof(mbs));
		while (bufend - bufnext >= MB_CUR_MAX) {
			clen = _mbrtowc(&wc, patnext, MB_LEN_MAX, &mbs);
			if (clen == (size_t)-1 || clen == (size_t)-2)
				return (GLOB_NOMATCH);
			else if (clen == 0)
				break;
			*bufnext++ = wc;
			patnext += clen;
		}
	} else {
		/* Protect the quoted characters. */
		memset(&mbs, 0, sizeof(mbs));
		while (bufend - bufnext >= MB_CUR_MAX) {
			if (*patnext == QUOTE) {
				if (*++patnext == EOS) {
					*bufnext++ = QUOTE | M_PROTECT;
					continue;
				}
				prot = M_PROTECT;
			} else
				prot = 0;
			clen = _mbrtowc(&wc, patnext, MB_LEN_MAX, &mbs);
			if (clen == (size_t)-1 || clen == (size_t)-2)
				return (GLOB_NOMATCH);
			else if (clen == 0)
				break;
			*bufnext++ = wc | prot;
			patnext += clen;
		}
	}
	*bufnext = EOS;

	if (flags & GLOB_BRACE)
		return globexp1(patbuf, pglob, &limit);
	else
		return glob0(patbuf, pglob, &limit);
}

/*
 * Expand recursively a glob {} pattern. When there is no more expansion
 * invoke the standard globbing routine to glob the rest of the magic
 * characters
 */
static int
globexp1(const Char *pattern, glob_t *pglob, size_t *limit)
{
	const Char* ptr = pattern;
	int rv;

	/* Protect a single {}, for find(1), like csh */
	if (pattern[0] == LBRACE && pattern[1] == RBRACE && pattern[2] == EOS)
		return glob0(pattern, pglob, limit);

	while ((ptr = g_strchr(ptr, LBRACE)) != NULL)
		if (!globexp2(ptr, pattern, pglob, &rv, limit))
			return rv;

	return glob0(pattern, pglob, limit);
}


/*
 * Recursive brace globbing helper. Tries to expand a single brace.
 * If it succeeds then it invokes globexp1 with the new pattern.
 * If it fails then it tries to glob the rest of the pattern and returns.
 */
static int
globexp2(const Char *ptr, const Char *pattern, glob_t *pglob, int *rv, size_t *limit)
{
	int		i;
	Char   *lm, *ls;
	const Char *pe, *pm, *pm1, *pl;
	Char	patbuf[MAXPATHLEN];

	/* copy part up to the brace */
	for (lm = patbuf, pm = pattern; pm != ptr; *lm++ = *pm++)
		continue;
	*lm = EOS;
	ls = lm;

	/* Find the balanced brace */
	for (i = 0, pe = ++ptr; *pe; pe++)
		if (*pe == LBRACKET) {
			/* Ignore everything between [] */
			for (pm = pe++; *pe != RBRACKET && *pe != EOS; pe++)
				continue;
			if (*pe == EOS) {
				/*
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pe = pm;
			}
		}
		else if (*pe == LBRACE)
			i++;
		else if (*pe == RBRACE) {
			if (i == 0)
				break;
			i--;
		}

	/* Non matching braces; just glob the pattern */
	if (i != 0 || *pe == EOS) {
		*rv = glob0(patbuf, pglob, limit);
		return 0;
	}

	for (i = 0, pl = pm = ptr; pm <= pe; pm++)
		switch (*pm) {
		case LBRACKET:
			/* Ignore everything between [] */
			for (pm1 = pm++; *pm != RBRACKET && *pm != EOS; pm++)
				continue;
			if (*pm == EOS) {
				/*
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pm = pm1;
			}
			break;

		case LBRACE:
			i++;
			break;

		case RBRACE:
			if (i) {
				i--;
				break;
			}
			/* FALLTHROUGH */
		case COMMA:
			if (i && *pm == COMMA)
				break;
			else {
				/* Append the current string */
				for (lm = ls; (pl < pm); *lm++ = *pl++)
					continue;
				/*
				 * Append the rest of the pattern after the
				 * closing brace
				 */
				for (pl = pe + 1; (*lm++ = *pl++) != EOS;)
					continue;

				/* Expand the current pattern */
#ifdef DEBUG
				qprintf("globexp2:", patbuf);
#endif
				*rv = globexp1(patbuf, pglob, limit);

				/* move after the comma, to the next string */
				pl = pm + 1;
			}
			break;

		default:
			break;
		}
	*rv = 0;
	return 0;
}



/*
 * expand tilde from the passwd file.
 */
static const Char *
globtilde(const Char *pattern, Char *patbuf, size_t patbuf_len, glob_t *pglob)
{
	struct passwd *pwd;
	char *h;
	const Char *p;
	Char *b, *eb;

	if (*pattern != TILDE || !(pglob->gl_flags & GLOB_TILDE))
		return pattern;

	/* 
	 * Copy up to the end of the string or / 
	 */
	eb = &patbuf[patbuf_len - 1];
	for (p = pattern + 1, h = (char *) patbuf;
		h < (char *)eb && *p && *p != SLASH; *h++ = *p++)
		continue;

	*h = EOS;

	if (((char *) patbuf)[0] == EOS) {
		/*
		 * handle a plain ~ or ~/ by expanding $HOME first (iff
		 * we're not running setuid or setgid) and then trying
		 * the password file
		 */
		if ((h = getenv("HOME")) == NULL) {
			return pattern;
		}
	}
	else {
		/*
		 * Expand a ~user
		 */
		if ((pwd = getpwnam((char*) patbuf)) == NULL)
			return pattern;
		else
			h = pwd->pw_dir;
	}

	/* Copy the home directory */
	for (b = patbuf; b < eb && *h; *b++ = *h++)
		continue;

	/* Append the rest of the pattern */
	while (b < eb && (*b++ = *p++) != EOS)
		continue;
	*b = EOS;

	return patbuf;
}


/*
 * The main glob() routine: compiles the pattern (optionally processing
 * quotes), calls glob1() to do the real pattern matching, and finally
 * sorts the list (unless unsorted operation is requested).	 Returns 0
 * if things went well, nonzero if errors occurred.
 */
static int
glob0(const Char *pattern, glob_t *pglob, size_t *limit)
{
	const Char *qpatnext;
	int err;
	size_t oldpathc;
	Char *bufnext, c, patbuf[MAXPATHLEN];

	qpatnext = globtilde(pattern, patbuf, MAXPATHLEN, pglob);
	oldpathc = pglob->gl_pathc;
	bufnext = patbuf;

	/* We don't need to check for buffer overflow any more. */
	while ((c = *qpatnext++) != EOS) {
		switch (c) {
		case LBRACKET:
			c = *qpatnext;
			if (c == NOT)
				++qpatnext;
			if (*qpatnext == EOS ||
				g_strchr(qpatnext+1, RBRACKET) == NULL) {
				*bufnext++ = LBRACKET;
				if (c == NOT)
					--qpatnext;
				break;
			}
			*bufnext++ = M_SET;
			if (c == NOT)
				*bufnext++ = M_NOT;
			c = *qpatnext++;
			do {
				*bufnext++ = CHAR(c);
				if (*qpatnext == RANGE &&
					(c = qpatnext[1]) != RBRACKET) {
					*bufnext++ = M_RNG;
					*bufnext++ = CHAR(c);
					qpatnext += 2;
				}
			} while ((c = *qpatnext++) != RBRACKET);
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_END;
			break;
		case QUESTION:
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_ONE;
			break;
		case STAR:
			pglob->gl_flags |= GLOB_MAGCHAR;
			/* collapse adjacent stars to one,
			 * to avoid exponential behavior
			 */
			if (bufnext == patbuf || bufnext[-1] != M_ALL)
				*bufnext++ = M_ALL;
			break;
		default:
			*bufnext++ = CHAR(c);
			break;
		}
	}
	*bufnext = EOS;
#ifdef DEBUG
	qprintf("glob0:", patbuf);
#endif

	if ((err = glob1(patbuf, pglob, limit)) != 0)
		return(err);

	/*
	 * If there was no match we are going to append the pattern
	 * if GLOB_NOCHECK was specified or if GLOB_NOMAGIC was specified
	 * and the pattern did not contain any magic characters
	 * GLOB_NOMAGIC is there just for compatibility with csh.
	 */
	if (pglob->gl_pathc == oldpathc) {
		if (((pglob->gl_flags & GLOB_NOCHECK) ||
			((pglob->gl_flags & GLOB_NOMAGIC) &&
			!(pglob->gl_flags & GLOB_MAGCHAR))))
			return(globextend(pattern, pglob, limit));
		else
			return(GLOB_NOMATCH);
	}
	if (!(pglob->gl_flags & GLOB_NOSORT))
		qsort(pglob->gl_pathv + pglob->gl_offs + oldpathc,
			pglob->gl_pathc - oldpathc, sizeof(char *), compare);
	return(0);
}

static int
compare(const void *p, const void *q)
{
	return(strcmp(*(char **)p, *(char **)q));
}

static int
glob1(Char *pattern, glob_t *pglob, size_t *limit)
{
	Char pathbuf[MAXPATHLEN];

	/* A null pathname is invalid -- POSIX 1003.1 sect. 2.4. */
	if (*pattern == EOS)
		return(0);
	return(glob2(pathbuf, pathbuf, pathbuf + MAXPATHLEN - 1,
		pattern, pglob, limit));
}

/*
 * The functions glob2 and glob3 are mutually recursive; there is one level
 * of recursion for each segment in the pattern that contains one or more
 * meta characters.
 */
static int
glob2(Char *pathbuf, Char *pathend, Char *pathend_last, Char *pattern,
	  glob_t *pglob, size_t *limit)
{
	struct stat sb;
	Char *p, *q;
	int anymeta;

	/*
	 * Loop over pattern segments until end of pattern or until
	 * segment with meta character found.
	 */
	for (anymeta = 0;;) {
		if (*pattern == EOS) {		/* End of pattern? */
			*pathend = EOS;
			if (g_lstat(pathbuf, &sb, pglob))
				return(0);

			if (((pglob->gl_flags & GLOB_MARK) &&
				pathend[-1] != SEP) && (S_ISDIR(sb.st_mode)
				|| (S_ISLNK(sb.st_mode) &&
				(g_stat(pathbuf, &sb, pglob) == 0) &&
				S_ISDIR(sb.st_mode)))) {
				if (pathend + 1 > pathend_last)
					return (GLOB_ABORTED);
				*pathend++ = SEP;
				*pathend = EOS;
			}
			++pglob->gl_matchc;
			return(globextend(pathbuf, pglob, limit));
		}

		/* Find end of next segment, copy tentatively to pathend. */
		q = pathend;
		p = pattern;
		while (*p != EOS && *p != SEP) {
			if (ismeta(*p))
				anymeta = 1;
			if (q + 1 > pathend_last)
				return (GLOB_ABORTED);
			*q++ = *p++;
		}

		if (!anymeta) {		/* No expansion, do next segment. */
			pathend = q;
			pattern = p;
			while (*pattern == SEP) {
				if (pathend + 1 > pathend_last)
					return (GLOB_ABORTED);
				*pathend++ = *pattern++;
			}
		} else			/* Need expansion, recurse. */
			return(glob3(pathbuf, pathend, pathend_last, pattern, p,
				pglob, limit));
	}
	/* NOTREACHED */
}

static int
glob3(Char *pathbuf, Char *pathend, Char *pathend_last,
	  Char *pattern, Char *restpattern,
	  glob_t *pglob, size_t *limit)
{
	struct dirent *dp;
	DIR *dirp;
	int err;
	char buf[MAXPATHLEN];

	/*
	 * The readdirfunc declaration can't be prototyped, because it is
	 * assigned, below, to two functions which are prototyped in glob.h
	 * and dirent.h as taking pointers to differently typed opaque
	 * structures.
	 */
	struct dirent *(*readdirfunc)();

	if (pathend > pathend_last)
		return (GLOB_ABORTED);
	*pathend = EOS;
	errno = 0;

	if ((dirp = g_opendir(pathbuf, pglob)) == NULL) {
		/* TODO: don't call for ENOENT or ENOTDIR? */
		if (pglob->gl_errfunc) {
			if (g_Ctoc(pathbuf, buf, sizeof(buf)))
				return (GLOB_ABORTED);
			if (pglob->gl_errfunc(buf, errno) ||
				pglob->gl_flags & GLOB_ERR)
				return (GLOB_ABORTED);
		}
		return(0);
	}

	err = 0;

	/* Search directory for matching names. */
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		readdirfunc = pglob->gl_readdir;
	else
		readdirfunc = readdir;
	while ((dp = (*readdirfunc)(dirp))) {
		char *sc;
		Char *dc;
		wchar_t wc;
		size_t clen;
		mbstate_t mbs;

		/* Initial DOT must be matched literally. */
		//140602, HQM: we want to match dots
		//if (dp->d_name[0] == DOT && *pattern != DOT)
		//	continue;
		//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","dp->d_name=%s",dp->d_name);
		memset(&mbs, 0, sizeof(mbs));
		dc = pathend;
		sc = dp->d_name;
		while (dc < pathend_last) {
			clen = _mbrtowc(&wc, sc, MB_LEN_MAX, &mbs);
			if (clen == (size_t)-1 || clen == (size_t)-2) {
				wc = *sc;
				clen = 1;
				memset(&mbs, 0, sizeof(mbs));
			}
			if ((*dc++ = wc) == EOS)
				break;
			sc += clen;
		}
		if (!match(pathend, pattern, restpattern)) {
			*pathend = EOS;
			continue;
		}
		err = glob2(pathbuf, --dc, pathend_last, restpattern,
			pglob, limit);
		if (err)
			break;
	}

	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		(*pglob->gl_closedir)(dirp);
	else
		closedir(dirp);
	return(err);
}


/*
 * Extend the gl_pathv member of a glob_t structure to accomodate a new item,
 * add the new item, and update gl_pathc.
 *
 * This assumes the BSD realloc, which only copies the block when its size
 * crosses a power-of-two boundary; for v7 realloc, this would cause quadratic
 * behavior.
 *
 * Return 0 if new item added, error code if memory couldn't be allocated.
 *
 * Invariant of the glob_t structure:
 *	Either gl_pathc is zero and gl_pathv is NULL; or gl_pathc > 0 and
 *	gl_pathv points to (gl_offs + gl_pathc + 1) items.
 */
static int
globextend(const Char *path, glob_t *pglob, size_t *limit)
{
	char **pathv;
	size_t i, newsize, len;
	char *copy;
	const Char *p;

	if (*limit && pglob->gl_pathc > *limit) {
		errno = 0;
		return (GLOB_NOSPACE);
	}

	newsize = sizeof(*pathv) * (2 + pglob->gl_pathc + pglob->gl_offs);
	pathv = pglob->gl_pathv ?
			realloc((char *)pglob->gl_pathv, newsize) :
			malloc(newsize);
	if (pathv == NULL) {
		if (pglob->gl_pathv) {
			free(pglob->gl_pathv);
			pglob->gl_pathv = NULL;
		}
		return(GLOB_NOSPACE);
	}

	if (pglob->gl_pathv == NULL && pglob->gl_offs > 0) {
		/* first time around -- clear initial gl_offs items */
		pathv += pglob->gl_offs;
		for (i = pglob->gl_offs + 1; --i > 0; )
			*--pathv = NULL;
	}
	pglob->gl_pathv = pathv;

	for (p = path; *p++;)
		continue;
	len = MB_CUR_MAX * (size_t)(p - path);	/* XXX overallocation */
	if ((copy = malloc(len)) != NULL) {
		if (g_Ctoc(path, copy, len)) {
			free(copy);
			return (GLOB_NOSPACE);
		}
		pathv[pglob->gl_offs + pglob->gl_pathc++] = copy;
	}
	pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;
	return(copy == NULL ? GLOB_NOSPACE : 0);
}

/*
 * pattern matching function for filenames.	 Each occurrence of the *
 * pattern causes a recursion level.
 */
static int
match(Char *name, Char *pat, Char *patend)
{
	int ok, negate_range;
	Char c, k;

	while (pat < patend) {
		c = *pat++;
		switch (c & M_MASK) {
		case M_ALL:
			if (pat == patend)
				return(1);
			do
				if (match(name, pat, patend))
					return(1);
			while (*name++ != EOS);
			return(0);
		case M_ONE:
			if (*name++ == EOS)
				return(0);
			break;
		case M_SET:
			ok = 0;
			if ((k = *name++) == EOS)
				return(0);
			if ((negate_range = ((*pat & M_MASK) == M_NOT)) != EOS)
				++pat;
			while (((c = *pat++) & M_MASK) != M_END)
				if ((*pat & M_MASK) == M_RNG) {
					if (CHAR(c) <= CHAR(k) && CHAR(k) <= CHAR(pat[1])) ok = 1;
					pat += 2;
				} else if (c == k)
					ok = 1;
			if (ok == negate_range)
				return(0);
			break;
		default:
			if (*name++ != c)
				return(0);
			break;
		}
	}
	return(*name == EOS);
}

/* Free allocated data belonging to a glob_t structure. */
void
globfree(glob_t *pglob)
{
	size_t i;
	char **pp;

	if (pglob->gl_pathv != NULL) {
		pp = pglob->gl_pathv + pglob->gl_offs;
		for (i = pglob->gl_pathc; i--; ++pp)
			if (*pp)
				free(*pp);
		free(pglob->gl_pathv);
		pglob->gl_pathv = NULL;
	}
}

static DIR *
g_opendir(Char *str, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (!*str)
		strcpy(buf, ".");
	else {
		if (g_Ctoc(str, buf, sizeof(buf)))
			return (NULL);
	}

	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return((*pglob->gl_opendir)(buf));

	//__android_log_print(ANDROID_LOG_ERROR,"STDOUT",">>> buf=%s, str=%ls",buf,str);
	return(opendir(buf));
}

static int
g_lstat(Char *fn, struct stat *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (g_Ctoc(fn, buf, sizeof(buf))) {
		errno = ENAMETOOLONG;
		return (-1);
	}
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return((*pglob->gl_lstat)(buf, sb));
	return(lstat(buf, sb));
}

static int
g_stat(Char *fn, struct stat *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (g_Ctoc(fn, buf, sizeof(buf))) {
		errno = ENAMETOOLONG;
		return (-1);
	}
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return((*pglob->gl_stat)(buf, sb));
	return(stat(buf, sb));
}

static const Char *
g_strchr(const Char *str, wchar_t ch)
{

	do {
		if (*str == ch)
			return (str);
	} while (*str++);
	return (NULL);
}

static int
g_Ctoc(const Char *str, char *buf, size_t len)
{
	mbstate_t mbs;
	size_t clen;

	memset(&mbs, 0, sizeof(mbs));
	while (len >= MB_CUR_MAX) {
		clen = _wcrtomb(buf, *str, &mbs);
		if (clen == (size_t)-1)
			return (1);
		if (*str == L'\0')
			return (0);
		str++;
		buf += clen;
		len -= clen;
	}
	return (1);
}

#ifdef DEBUG
static void
qprintf(const char *str, Char *s)
{
	Char *p;

	(void)printf("%s:\n", str);
	for (p = s; *p; p++)
		(void)printf("%c", CHAR(*p));
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", *p & M_PROTECT ? '"' : ' ');
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", ismeta(*p) ? '_' : ' ');
	(void)printf("\n");
}
#endif
////////////////////////////////
#endif

EXPORT int osal_PollPipe(int fd){
	struct pollfd pfd;
	memset(&pfd,0,sizeof(pfd));
	pfd.fd=fd;
	pfd.events = POLLIN;
	return poll(&pfd,1,0)==1;
}

EXPORT iptr osal_GetFileSize(int fd){
	struct stat sb;
	sb.st_size=0;
	fstat(fd, &sb);
	return (iptr)sb.st_size;
}

EXPORT i64 osal_GetFileSize64(int fd){
	struct stat sb;
	sb.st_size=0;
	fstat(fd, &sb);
	return (i64)sb.st_size;
}

EXPORT void* osal_BeginFind(char* fn_pattern){
	//GLOB_ERR
	glob_t* ret=calloc(1,sizeof(glob_t));
	int rvalue=glob(fn_pattern, GLOB_NOESCAPE,NULL,ret);
	//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","glob pattern=%s rvalue=%d pathc=%d",fn_pattern,rvalue,ret->gl_pathc);
	if(rvalue==GLOB_NOMATCH){
		ret->gl_pathc=0;
	}
	ret->gl_offs=0;
	return (void*)ret;
}

EXPORT int osal_EndFind(void* handle){
	((glob_t*)handle)->gl_offs=0;
	globfree((glob_t*)handle);
	free(handle);
	return 1;
}

#define OSAL_CP_PIPE_STDIN 1
#define OSAL_CP_PIPE_STDOUT 2
#define OSAL_CP_PIPE_STDERR 4
EXPORT int osal_CreateProcess(int* ret, char** zargv,char* spath,int flags){
	int pipes[4];
	int pid=0;
	pipes[0]=-1;pipes[1]=-1;
	pipes[2]=-1;pipes[3]=-1;
	//pipes[4]=-1;pipes[5]=-1;
	if(flags&OSAL_CP_PIPE_STDIN){pipe(pipes+0);}
	if(flags&(OSAL_CP_PIPE_STDOUT|OSAL_CP_PIPE_STDERR)){pipe(pipes+2);}
	//if(flags&OSAL_CP_PIPE_STDERR){pipe(pipes+4);}
	pid=fork();
	if(pid==0){
		chdir(spath);
		//child
		if(flags&OSAL_CP_PIPE_STDIN){dup2(pipes[0+0], STDIN_FILENO);close(pipes[0+0]);close(pipes[0+1]);}
		if(flags&OSAL_CP_PIPE_STDOUT){dup2(pipes[2+1], STDOUT_FILENO);}
		if(flags&OSAL_CP_PIPE_STDERR){dup2(pipes[2+1], STDERR_FILENO);}
		if(flags&(OSAL_CP_PIPE_STDOUT|OSAL_CP_PIPE_STDERR)){close(pipes[2+0]);close(pipes[2+1]);}
		execvp(zargv[0],zargv);
		_exit(1);
	}else{
		//parent
		if(pid<0){return 0;}
		ret[0]=pid;
		ret[1]=-1;
		ret[2]=-1;
		if(flags&OSAL_CP_PIPE_STDIN){close(pipes[0+0]);ret[1]=pipes[0+1];}
		if(flags&(OSAL_CP_PIPE_STDOUT|OSAL_CP_PIPE_STDERR)){close(pipes[2+1]);ret[2]=pipes[2+0];}
	}
	return 1;
}

EXPORT int osal_GetExitCodeProcess(int pid){
	int stat_val=0;
	int pid_ret=waitpid(pid,&stat_val,WNOHANG);
	if(pid_ret==pid){
		if(WIFEXITED(stat_val)){return -1;}
		return WEXITSTATUS(stat_val);
	}else if(pid_ret==0){
		//it's still running
		return -1;
	}else{
		//assume it errored out
		return 1;
	}
}

EXPORT int osal_TerminateProcess(int pid){
	return kill(pid,SIGKILL)==0;
}

//#define FILE_ATTRIBUTE_READONLY 0x00000001
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010

typedef struct _OSAL_TFileInfo{
	i64 created_time;
	i64 last_modified_time;
	i64 size;
	int attr;
}OSAL_TFileInfo;

EXPORT int osal_FindNext(void* handle, char* fn,OSAL_TFileInfo* fi){
	glob_t* handleg=(glob_t*)handle;
	struct stat sb;
	if(handleg->gl_offs>=handleg->gl_pathc)return 0;
	fn[1023]=0;
	strncpy(fn,handleg->gl_pathv[handleg->gl_offs],1023);
	memset(&sb,0,sizeof(sb));
	stat(fn,&sb);
	handleg->gl_offs++;
	fi->created_time=(i64)sb.st_ctime;
	fi->last_modified_time=(i64)sb.st_mtime;
	fi->size=(i64)sb.st_size;
	fi->attr=(S_ISDIR(sb.st_mode)?FILE_ATTRIBUTE_DIRECTORY:0);
	return 1;
}

EXPORT int osal_GetFileAttributes(char* fn){
	struct stat sb;
	sb.st_size=0;
	if(stat(fn, &sb)!=0)return -1;
	return S_ISDIR(sb.st_mode)?FILE_ATTRIBUTE_DIRECTORY:0;
}

#define STD_INPUT_HANDLE (0xfffffff6)
#define STD_OUTPUT_HANDLE (0xfffffff5)
#define STD_ERROR_HANDLE (0xfffffff4)
EXPORT FILE* osal_GetStdHandle(int val){
	if(val==STD_INPUT_HANDLE)return stdin;
	if(val==STD_OUTPUT_HANDLE)return stdout;
	if(val==STD_ERROR_HANDLE)return stderr;
	return NULL;
}

EXPORT iptr osal_GetUnixPathMax(){return PATH_MAX;}

EXPORT void *osal_mmap(void *addr, iptr length, int prot, int flags,int fd){
	void* ret=mmap(addr,length,prot,flags,fd,0);
	//__android_log_print(ANDROID_LOG_ERROR,"STDOUT","mmap failed errno = %d\n", errno);
	return ret;
}

EXPORT int osal_errno(){return errno;}
#endif

static char* g_dummy_argv0="";
static char** g_argv=&g_dummy_argv0;
static int g_argc=1;
EXPORT void osal_SetCommandLine(int argc,char** argv){
	g_argv=argv;
	g_argc=argc;
}
EXPORT int osal_GetCommandLine(char*** pargv){
	*pargv=g_argv;
	return g_argc;
}

#ifdef NEED_MAIN_WRAPPING
#if defined(_WIN32)
//those platforms don't need SDL_main
#undef main
#endif
extern int real_main();
int main(int argc,char** argv){
	g_argv=argv;
	g_argc=argc;
	return real_main();
}
#endif
