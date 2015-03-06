#ifndef __FILEDLG_H
#define __FILEDLG_H

#ifdef _WIN32
//the windows version uses utf16
int osal_PickImageWin(short* buf);
#else
char* osal_PickImage(int* out_len);
#endif

#endif
