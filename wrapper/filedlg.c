#include "wrapper_defines.h"
#include "filedlg.h"

#ifdef _WIN32
#include <windows.h>
//my pictures directory in buf
int osal_PickImageWin(short* buf){
	OPENFILENAMEW ofn;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.lpstrFilter=L"Image files\0*.PNG;*.JPG;*.JPEG;*.BMP;*.TIF;*.TIFF;*.TGA\0All files\0*.*\0";
	ofn.lpstrDefExt=L"png";
	ofn.lpstrFile=buf;
	ofn.nMaxFile=260;
	ofn.Flags=OFN_NOCHANGEDIR;
	return GetOpenFileNameW(&ofn);
}

int osal_DoFileDialogWin(short* buf, short* filter,short* def_ext,int is_save){
	OPENFILENAMEW ofn;
	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(ofn);
	ofn.lpstrFilter=filter;
	ofn.lpstrDefExt=def_ext;
	ofn.lpstrFile=buf;
	ofn.nMaxFile=260;
	if(is_save){
		ofn.Flags=OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
		return GetSaveFileNameW(&ofn);
	}else{
		ofn.Flags=OFN_NOCHANGEDIR;
		return GetOpenFileNameW(&ofn);
	}
}
#endif
