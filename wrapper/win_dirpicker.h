#pragma once
#ifndef __WIN_DIRPICKER_H
#define __WIN_DIRPICKER_H

#ifdef __cplusplus
extern "C"{
#endif
void winDirPickerInit();
short* winDirPickerCall();
void winDirPickerFree(short* s);
#ifdef __cplusplus
}
#endif

#endif
