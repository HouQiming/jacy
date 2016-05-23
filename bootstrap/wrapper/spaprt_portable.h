#ifndef __SPAPRT_PORTABLE_H
#define __SPAPRT_PORTABLE_H
#if S7_POINTER_BITS==64
typedef long long iptr;
#else
typedef int iptr;
#endif

typedef struct _TLineInfoItemEx{
	const char* s;
	const struct _TLineInfoItemEx* next;
}TLineInfoItemEx;

typedef struct _TCallStackItemEx{
	struct _TCallStackItemEx* dad;
	const char* fnn;
	const TLineInfoItemEx* li;
}TCallStackItemEx;

#ifdef __cplusplus
extern "C"{
#endif
void spapDebugV2Start();
void spapPushCallStack(TCallStackItemEx* p);
void spapPopCallStack();
void spapReportError(char* s);
void spapReportErrorf(char* fmt,...);
void osal_WriteLog(char* buf);
void spapBlockYielding();
void spapUnblockYielding();
void spapInitCoroutines();
void spapStartRunningCoroutines();
void spapStopRunningCoroutines();

typedef struct _OSAL_TFileInfo{
	long long created_time;
	long long last_modified_time;
	long long size;
	int attr;
}OSAL_TFileInfo;

int osal_PollPipe(int fd);
iptr osal_GetFileSize(int fd);
long long osal_GetFileSize64(int fd);
void* osal_BeginFind(char* fn_pattern);
int osal_EndFind(void* handle);
int osal_FindNext(void* handle, char* fn,OSAL_TFileInfo* fi);
int osal_GetFileAttributes(char* fn);
iptr osal_GetUnixPathMax();
void *osal_mmap(void *addr, iptr length, int prot, int flags,int fd);
int osal_errno();

int osal_CreateProcess(int* ret, char** zargv,char* spath,int flags);

void* osal_mmap_res_zip(iptr* psize);
char* osal_getStoragePath();

void osal_LinuxXIOErrorWorkaround();

int osal_GetExitCodeProcess(int pid);
int osal_TerminateProcess(int pid);

#ifdef __cplusplus
}
#endif

#endif
