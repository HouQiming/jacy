#ifndef __FILEDLG_H
#define __FILEDLG_H

#ifdef _WIN32
//the windows version uses utf16
int osal_PickImageWin(short* buf);
int osal_DoFileDialogWin(short* buf, short* filter,short* def_ext,int is_save);
#else
//char* osal_PickImage(int* out_len);
//int osal_DoFileDialog(char* buf, char* filter,char* def_ext,int is_save)
#endif

#endif
