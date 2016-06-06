#include <windows.h>
#include <shobjidl.h>
#include <stdio.h>

static ITaskbarList3 *ptbl=NULL;
extern "C" int SetTaskbarProgress(HWND hwnd,float value){
	HRESULT hr=0;
	if(!ptbl){
		CoInitializeEx(NULL,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
		hr = CoCreateInstance(CLSID_TaskbarList, 
										NULL, 
										CLSCTX_INPROC_SERVER, 
										IID_PPV_ARGS(&ptbl));
		if(!SUCCEEDED(hr)){
			ptbl=NULL;
			return 0;
		}
	}
	if(!ptbl){return 0;}
	hr=ptbl->SetProgressState(hwnd,(TBPFLAG)(value>0.f?2:0));
	if(!SUCCEEDED(hr)){return 0;}
	hr=ptbl->SetProgressValue(hwnd,(ULONGLONG)(value*1048576.f),(ULONGLONG)1048576);
	if(!SUCCEEDED(hr)){return 0;}
	return 1;
}
