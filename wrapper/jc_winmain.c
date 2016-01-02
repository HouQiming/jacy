#include <windows.h>
#include <malloc.h>

extern int main(int,char**);

#define PUSH_RET(c) ((ret?(ret[p]=c):(char)0),p++)
static size_t Utf16ToUtf8Raw(LPWSTR s0,char* ret){
	size_t p=0;
	int surrogate_high=0;
	LPWSTR I=s0;
	for(I=s0;*I;I++){
		int chi=(int)((unsigned short)(*I));
		if((unsigned int)(chi-0xd800)<0x400u){
			//surrogate pair - 1st
			surrogate_high=(chi&0x3ff);
			continue;
		}else if((unsigned int)(chi-0xdc00)<0x400u){
			//surrogate pair - 2nd
			chi=(surrogate_high<<10)+(chi&0x3ff)+0x10000;
			surrogate_high=0;
			PUSH_RET((char)(((chi>>18)&0xf)+0xf0));
			PUSH_RET((char)(0x80+((chi>>12)&63)));
			PUSH_RET((char)(0x80+((chi>>6)&63)));
			PUSH_RET((char)(0x80+(chi&63)));
			continue;
		}if(chi>=2048){
			PUSH_RET((char)(((chi>>12)&0xf)+0xe0));
			PUSH_RET((char)(0x80+((chi>>6)&63)));
			PUSH_RET((char)(0x80+(chi&63)));
		}else if(chi>=128){
			PUSH_RET((char)((chi>>6)+0xc0));
			PUSH_RET((char)(0x80+(chi&63)));
		}else{
			PUSH_RET((char)(chi));
		}
	}
	PUSH_RET(0);
	return p;
}

static char* Utf16ToUtf8(LPWSTR s0){
	size_t sz=Utf16ToUtf8Raw(s0,NULL);
	char* ret=(char*)calloc(1,sz);
	Utf16ToUtf8Raw(s0,ret);
	return ret;
}

int CALLBACK wWinMain(HINSTANCE instance, HINSTANCE prev_instance, LPWSTR command_line,int d){
	int argc=0;
	int i=0;
	char** argv=NULL;
	LPWSTR* utf16_argv=CommandLineToArgvW(GetCommandLineW(),&argc);
	if(argc){
		argv=(char**)calloc(argc+1,sizeof(char*));
		for(i=0;i<argc;i++){
			argv[i]=Utf16ToUtf8(utf16_argv[i]);
		}
	}
	return main(argc,argv);
}
