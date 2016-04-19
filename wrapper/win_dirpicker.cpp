#include <windows.h>
#include <shobjidl.h>

extern "C" void winDirPickerInit(){
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
}

extern "C" LPWSTR winDirPickerCall(){
	IFileDialog *pfd=NULL;
	LPWSTR g_path=NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
	{
		DWORD dwOptions=0;
		if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
		}
		if (SUCCEEDED(pfd->Show(NULL)))
		{
			IShellItem *psi=NULL;
			if (SUCCEEDED(pfd->GetResult(&psi)))
			{
				if(SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &g_path)))
				{
					//do nothing;
				}
				psi->Release();
			}
		}
		pfd->Release();
	}
	return g_path;
}

extern "C" void winDirPickerFree(LPWSTR s){
	CoTaskMemFree(s);
}
