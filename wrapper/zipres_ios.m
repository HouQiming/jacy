#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#include "sdl.h"
#include "wrapper_defines.h"
#include <fcntl.h>
#include <sys/mman.h>

static void* g_res_zip=NULL;
static iptr g_res_size=0;
extern iptr osal_GetFileSize(int fd);
EXPORT void* osal_mmap_res_zip(iptr* psize){
	if(!g_res_size){
		NSString *str=[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"reszip.bundle"];
		const char *fn = [str UTF8String];
		int fd=open(fn,O_RDONLY);
		//printf("%s %d\n",fn,fd);
		if(fd==-1){
			g_res_size=-1;
		}else{
			g_res_size=osal_GetFileSize(fd);
			g_res_zip=mmap(NULL,g_res_size,PROT_READ,MAP_SHARED,fd,0);
			//printf("mapped: %08x %d\n",g_res_zip,g_res_size);
		}
	}
	if(g_res_size<0){*psize=0;}else{*psize=g_res_size;}
	return g_res_zip;
}
