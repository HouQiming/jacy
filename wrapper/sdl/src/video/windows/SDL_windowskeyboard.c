/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_WINDOWS

#include "SDL_windowsvideo.h"

#include "../../events/SDL_keyboard_c.h"
#include "../../events/scancodes_windows.h"

#include <imm.h>
#include <oleauto.h>

#ifndef SDL_DISABLE_WINDOWS_IME
static void IME_Init(SDL_VideoData *videodata, HWND hwnd);
static void IME_Enable(SDL_VideoData *videodata, HWND hwnd);
static void IME_Disable(SDL_VideoData *videodata, HWND hwnd);
static void IME_Quit(SDL_VideoData *videodata);
#endif /* !SDL_DISABLE_WINDOWS_IME */

#ifndef MAPVK_VK_TO_VSC
#define MAPVK_VK_TO_VSC     0
#endif
#ifndef MAPVK_VSC_TO_VK
#define MAPVK_VSC_TO_VK     1
#endif
#ifndef MAPVK_VK_TO_CHAR
#define MAPVK_VK_TO_CHAR    2
#endif

/* Alphabetic scancodes for PC keyboards */
void
WIN_InitKeyboard(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;

    data->ime_com_initialized = SDL_FALSE;
    data->ime_threadmgr = 0;
    data->ime_initialized = SDL_FALSE;
    data->ime_enabled = SDL_FALSE;
    data->ime_available = SDL_FALSE;
    data->ime_hwnd_main = 0;
    data->ime_hwnd_current = 0;
    data->ime_himc = 0;
    data->ime_composition[0] = 0;
    data->ime_readingstring[0] = 0;
    data->ime_cursor = 0;

    data->ime_candlist = SDL_FALSE;
    SDL_memset(data->ime_candidates, 0, sizeof(data->ime_candidates));
    data->ime_candcount = 0;
    data->ime_candref = 0;
    data->ime_candsel = 0;
    data->ime_candpgsize = 0;
    data->ime_candlistindexbase = 0;
    data->ime_candvertical = SDL_TRUE;

    data->ime_dirty = SDL_FALSE;
    SDL_memset(&data->ime_rect, 0, sizeof(data->ime_rect));
    SDL_memset(&data->ime_candlistrect, 0, sizeof(data->ime_candlistrect));
    data->ime_winwidth = 0;
    data->ime_winheight = 0;

    data->ime_hkl = 0;
    data->ime_himm32 = 0;
    data->GetReadingString = 0;
    data->ShowReadingWindow = 0;
    data->ImmLockIMC = 0;
    data->ImmUnlockIMC = 0;
    data->ImmLockIMCC = 0;
    data->ImmUnlockIMCC = 0;
    data->ime_uiless = SDL_FALSE;
    data->ime_threadmgrex = 0;
    data->ime_uielemsinkcookie = TF_INVALID_COOKIE;
    data->ime_alpnsinkcookie = TF_INVALID_COOKIE;
    data->ime_openmodesinkcookie = TF_INVALID_COOKIE;
    data->ime_convmodesinkcookie = TF_INVALID_COOKIE;
    data->ime_uielemsink = 0;
    data->ime_ippasink = 0;

    WIN_UpdateKeymap();

    SDL_SetScancodeName(SDL_SCANCODE_APPLICATION, "Menu");
    SDL_SetScancodeName(SDL_SCANCODE_LGUI, "Left Windows");
    SDL_SetScancodeName(SDL_SCANCODE_RGUI, "Right Windows");
}

void
WIN_UpdateKeymap()
{
    int i;
    SDL_Scancode scancode;
    SDL_Keycode keymap[SDL_NUM_SCANCODES];

    SDL_GetDefaultKeymap(keymap);

    for (i = 0; i < SDL_arraysize(windows_scancode_table); i++) {
        int vk;
        /* Make sure this scancode is a valid character scancode */
        scancode = windows_scancode_table[i];
        if (scancode == SDL_SCANCODE_UNKNOWN ) {
            continue;
        }

        /* If this key is one of the non-mappable keys, ignore it */
        /* Don't allow the number keys right above the qwerty row to translate or the top left key (grave/backquote) */
        /* Not mapping numbers fixes the French layout, giving numeric keycodes for the number keys, which is the expected behavior */
        if ((keymap[scancode] & SDLK_SCANCODE_MASK) ||
            scancode == SDL_SCANCODE_GRAVE ||
            (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0) ) {
            continue;
        }

        vk =  MapVirtualKey(i, MAPVK_VSC_TO_VK);
        if ( vk ) {
            int ch = (MapVirtualKey( vk, MAPVK_VK_TO_CHAR ) & 0x7FFF);
            if ( ch ) {
                if ( ch >= 'A' && ch <= 'Z' ) {
                    keymap[scancode] =  SDLK_a + ( ch - 'A' );
                } else {
                    keymap[scancode] = ch;
                }
            }
        }
    }

    SDL_SetKeymap(0, keymap, SDL_NUM_SCANCODES);
}

void
WIN_QuitKeyboard(_THIS)
{
#ifndef SDL_DISABLE_WINDOWS_IME
    IME_Quit((SDL_VideoData *)_this->driverdata);
#endif
}

void
WIN_StartTextInput(_THIS)
{
#ifndef SDL_DISABLE_WINDOWS_IME
    SDL_Window *window = SDL_GetKeyboardFocus();
    if (window) {
        HWND hwnd = ((SDL_WindowData *) window->driverdata)->hwnd;
        SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;
        SDL_GetWindowSize(window, &videodata->ime_winwidth, &videodata->ime_winheight);
        IME_Init(videodata, hwnd);
        IME_Enable(videodata, hwnd);
    }
#endif /* !SDL_DISABLE_WINDOWS_IME */
}

void
WIN_StopTextInput(_THIS)
{
#ifndef SDL_DISABLE_WINDOWS_IME
    SDL_Window *window = SDL_GetKeyboardFocus();
    if (window) {
        HWND hwnd = ((SDL_WindowData *) window->driverdata)->hwnd;
        SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;
        IME_Init(videodata, hwnd);
        IME_Disable(videodata, hwnd);
    }
#endif /* !SDL_DISABLE_WINDOWS_IME */
}

void
WIN_SetTextInputRect(_THIS, SDL_Rect *rect)
{
    SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;
    HIMC himc = 0;

    if (!rect) {
        SDL_InvalidParamError("rect");
        return;
    }

    videodata->ime_rect = *rect;
    
    himc = ImmGetContext(videodata->ime_hwnd_current);
    if (himc) {
        COMPOSITIONFORM cf;
        CANDIDATEFORM cf2;
        int i;
        SDL_memset(&cf,0,sizeof(cf));
        cf.dwStyle=CFS_POINT;
        cf.ptCurrentPos.x=rect->x;
        cf.ptCurrentPos.y=rect->y;
        cf.rcArea.left=rect->x;
        cf.rcArea.top=rect->y;
        cf.rcArea.right=rect->x+rect->w;
        cf.rcArea.bottom=rect->y+rect->h;
        //if(rect->y!=376){
        //	printf("%d %d %d %d\n",rect->x,rect->y,rect->w,rect->h);fflush(stdout);
        //}
        /*if(videodata->ime_hwnd_current){
	        CreateCaret(videodata->ime_hwnd_current,NULL,1,1);
	        SetCaretPos(rect->x,rect->y);
	        ShowCaret(videodata->ime_hwnd_current);
        }*/
        for(i=0;i<4;i++){
        	SDL_memset(&cf2,0,sizeof(cf2));
        	cf2.dwStyle=CFS_CANDIDATEPOS;
        	cf2.dwIndex=i;
        	cf2.ptCurrentPos.x=rect->x;
        	cf2.ptCurrentPos.y=rect->y;
        	ImmSetCandidateWindow(himc,&cf2);
        }
        ImmSetCompositionWindow(himc,&cf);
        ImmReleaseContext(videodata->ime_hwnd_current, himc);
        /////////////////////////
        //we need to update it
        videodata->ime_himc = himc;
    }
}

#ifdef SDL_DISABLE_WINDOWS_IME


SDL_bool
IME_HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM *lParam, SDL_VideoData *videodata)
{
    return SDL_FALSE;
}

void IME_Present(SDL_VideoData *videodata)
{
}

#else

#ifdef __GNUC__
#undef DEFINE_GUID
#define DEFINE_GUID(n,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) static const GUID n = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
DEFINE_GUID(IID_ITfInputProcessorProfileActivationSink,        0x71C6E74E,0x0F28,0x11D8,0xA8,0x2A,0x00,0x06,0x5B,0x84,0x43,0x5C);
DEFINE_GUID(IID_ITfUIElementSink,                              0xEA1EA136,0x19DF,0x11D7,0xA6,0xD2,0x00,0x06,0x5B,0x84,0x43,0x5C);
DEFINE_GUID(GUID_TFCAT_TIP_KEYBOARD,                           0x34745C63,0xB2F0,0x4784,0x8B,0x67,0x5E,0x12,0xC8,0x70,0x1A,0x31);
DEFINE_GUID(IID_ITfSource,                                     0x4EA48A35,0x60AE,0x446F,0x8F,0xD6,0xE6,0xA8,0xD8,0x24,0x59,0xF7);
DEFINE_GUID(IID_ITfUIElementMgr,                               0xEA1EA135,0x19DF,0x11D7,0xA6,0xD2,0x00,0x06,0x5B,0x84,0x43,0x5C);
DEFINE_GUID(IID_ITfCandidateListUIElement,                     0xEA1EA138,0x19DF,0x11D7,0xA6,0xD2,0x00,0x06,0x5B,0x84,0x43,0x5C);
DEFINE_GUID(IID_ITfReadingInformationUIElement,                0xEA1EA139,0x19DF,0x11D7,0xA6,0xD2,0x00,0x06,0x5B,0x84,0x43,0x5C);
DEFINE_GUID(IID_ITfThreadMgr,                                  0xAA80E801,0x2021,0x11D2,0x93,0xE0,0x00,0x60,0xB0,0x67,0xB8,0x6E);
DEFINE_GUID(CLSID_TF_ThreadMgr,                                0x529A9E6B,0x6587,0x4F23,0xAB,0x9E,0x9C,0x7D,0x68,0x3E,0x3C,0x50);
DEFINE_GUID(IID_ITfThreadMgrEx,                                0x3E90ADE3,0x7594,0x4CB0,0xBB,0x58,0x69,0x62,0x8F,0x5F,0x45,0x8C);
#endif

#define LANG_CHT MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)

#define MAKEIMEVERSION(major,minor) ((DWORD) (((BYTE)(major) << 24) | ((BYTE)(minor) << 16) ))
#define IMEID_VER(id) ((id) & 0xffff0000)
#define IMEID_LANG(id) ((id) & 0x0000ffff)

#define CHT_HKL_DAYI            ((HKL)0xE0060404)
#define CHT_HKL_NEW_PHONETIC    ((HKL)0xE0080404)
#define CHT_HKL_NEW_CHANG_JIE   ((HKL)0xE0090404)
#define CHT_HKL_NEW_QUICK       ((HKL)0xE00A0404)
#define CHT_HKL_HK_CANTONESE    ((HKL)0xE00B0404)
#define CHT_IMEFILENAME1        "TINTLGNT.IME"
#define CHT_IMEFILENAME2        "CINTLGNT.IME"
#define CHT_IMEFILENAME3        "MSTCIPHA.IME"
#define IMEID_CHT_VER42         (LANG_CHT | MAKEIMEVERSION(4, 2))
#define IMEID_CHT_VER43         (LANG_CHT | MAKEIMEVERSION(4, 3))
#define IMEID_CHT_VER44         (LANG_CHT | MAKEIMEVERSION(4, 4))
#define IMEID_CHT_VER50         (LANG_CHT | MAKEIMEVERSION(5, 0))
#define IMEID_CHT_VER51         (LANG_CHT | MAKEIMEVERSION(5, 1))
#define IMEID_CHT_VER52         (LANG_CHT | MAKEIMEVERSION(5, 2))
#define IMEID_CHT_VER60         (LANG_CHT | MAKEIMEVERSION(6, 0))
#define IMEID_CHT_VER_VISTA     (LANG_CHT | MAKEIMEVERSION(7, 0))

#define CHS_HKL                 ((HKL)0xE00E0804)
#define CHS_IMEFILENAME1        "PINTLGNT.IME"
#define CHS_IMEFILENAME2        "MSSCIPYA.IME"
#define IMEID_CHS_VER41         (LANG_CHS | MAKEIMEVERSION(4, 1))
#define IMEID_CHS_VER42         (LANG_CHS | MAKEIMEVERSION(4, 2))
#define IMEID_CHS_VER53         (LANG_CHS | MAKEIMEVERSION(5, 3))

#define LANG() LOWORD((videodata->ime_hkl))
#define PRIMLANG() ((WORD)PRIMARYLANGID(LANG()))
#define SUBLANG() SUBLANGID(LANG())

static void IME_UpdateInputLocale(SDL_VideoData *videodata);
static void IME_ClearComposition(SDL_VideoData *videodata);
static void IME_SetWindow(SDL_VideoData* videodata, HWND hwnd);
static void IME_SendEditingEvent(SDL_VideoData *videodata);

static void
IME_Init(SDL_VideoData *videodata, HWND hwnd)
{
    if (videodata->ime_initialized)
        return;

    videodata->ime_hwnd_main = hwnd;
    videodata->ime_initialized = SDL_TRUE;
    videodata->ime_himm32 = SDL_LoadObject("imm32.dll");
    if (!videodata->ime_himm32) {
        videodata->ime_available = SDL_FALSE;
        return;
    }

    IME_SetWindow(videodata, hwnd);
    videodata->ime_himc = ImmGetContext(hwnd);
    ImmReleaseContext(hwnd, videodata->ime_himc);
    if (!videodata->ime_himc) {
        videodata->ime_available = SDL_FALSE;
        IME_Disable(videodata, hwnd);
        return;
    }
    videodata->ime_available = SDL_TRUE;
    IME_UpdateInputLocale(videodata);
    videodata->ime_uiless = SDL_FALSE;
    IME_UpdateInputLocale(videodata);
    IME_Disable(videodata, hwnd);
}

static void
IME_Enable(SDL_VideoData *videodata, HWND hwnd)
{
    if (!videodata->ime_initialized || !videodata->ime_hwnd_current)
        return;

    if (!videodata->ime_available) {
        IME_Disable(videodata, hwnd);
        return;
    }
    if (videodata->ime_hwnd_current == videodata->ime_hwnd_main)
        ImmAssociateContext(videodata->ime_hwnd_current, videodata->ime_himc);

    videodata->ime_enabled = SDL_TRUE;
    IME_UpdateInputLocale(videodata);
}

static void
IME_Disable(SDL_VideoData *videodata, HWND hwnd)
{
    if (!videodata->ime_initialized || !videodata->ime_hwnd_current)
        return;

    IME_ClearComposition(videodata);
    if (videodata->ime_hwnd_current == videodata->ime_hwnd_main)
        ImmAssociateContext(videodata->ime_hwnd_current, (HIMC)0);

    videodata->ime_enabled = SDL_FALSE;
}

static void
IME_Quit(SDL_VideoData *videodata)
{
    if (!videodata->ime_initialized)
        return;

    if (videodata->ime_hwnd_main)
        ImmAssociateContext(videodata->ime_hwnd_main, videodata->ime_himc);

    videodata->ime_hwnd_main = 0;
    videodata->ime_himc = 0;
    if (videodata->ime_himm32) {
        SDL_UnloadObject(videodata->ime_himm32);
        videodata->ime_himm32 = 0;
    }
    if (videodata->ime_threadmgr) {
        videodata->ime_threadmgr->lpVtbl->Release(videodata->ime_threadmgr);
        videodata->ime_threadmgr = 0;
    }
    if (videodata->ime_com_initialized) {
        WIN_CoUninitialize();
        videodata->ime_com_initialized = SDL_FALSE;
    }
    videodata->ime_initialized = SDL_FALSE;
}

static void
IME_InputLangChanged(SDL_VideoData *videodata)
{
    UINT lang = PRIMLANG();
    IME_UpdateInputLocale(videodata);
    if (!videodata->ime_uiless)
        videodata->ime_candlistindexbase = (videodata->ime_hkl == CHT_HKL_DAYI) ? 0 : 1;

    if (lang != PRIMLANG()) {
        IME_ClearComposition(videodata);
    }
    
    //for WIN+SPACE
    SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_LGUI);
    SDL_SendKeyboardKey(SDL_RELEASED,SDL_SCANCODE_RGUI);
}

static void
IME_SetWindow(SDL_VideoData* videodata, HWND hwnd)
{
    videodata->ime_hwnd_current = hwnd;
    if (videodata->ime_threadmgr) {
        struct ITfDocumentMgr *document_mgr = 0;
        if (SUCCEEDED(videodata->ime_threadmgr->lpVtbl->AssociateFocus(videodata->ime_threadmgr, hwnd, NULL, &document_mgr))) {
            if (document_mgr)
                document_mgr->lpVtbl->Release(document_mgr);
        }
    }
}

static void
IME_UpdateInputLocale(SDL_VideoData *videodata)
{
    static HKL hklprev = 0;
    videodata->ime_hkl = GetKeyboardLayout(0);
    if (hklprev == videodata->ime_hkl)
        return;

    hklprev = videodata->ime_hkl;
    switch (PRIMLANG()) {
    case LANG_CHINESE:
        videodata->ime_candvertical = SDL_TRUE;
        if (SUBLANG() == SUBLANG_CHINESE_SIMPLIFIED)
            videodata->ime_candvertical = SDL_FALSE;

        break;
    case LANG_JAPANESE:
        videodata->ime_candvertical = SDL_TRUE;
        break;
    case LANG_KOREAN:
        videodata->ime_candvertical = SDL_FALSE;
        break;
    }
}

static void
IME_ClearComposition(SDL_VideoData *videodata)
{
    HIMC himc = 0;
    if (!videodata->ime_initialized)
        return;

    himc = ImmGetContext(videodata->ime_hwnd_current);
    if (!himc)
        return;

    ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    if (videodata->ime_uiless)
        ImmSetCompositionString(himc, SCS_SETSTR, TEXT(""), sizeof(TCHAR), TEXT(""), sizeof(TCHAR));

    ImmNotifyIME(himc, NI_CLOSECANDIDATE, 0, 0);
    ImmReleaseContext(videodata->ime_hwnd_current, himc);
    SDL_SendEditingText("", 0, 0);
}

static void
IME_GetCompositionString(SDL_VideoData *videodata, HIMC himc, DWORD string)
{
    LONG length = ImmGetCompositionStringW(himc, string, videodata->ime_composition, sizeof(videodata->ime_composition) - sizeof(videodata->ime_composition[0]));
    if (length < 0)
        length = 0;

    length /= sizeof(videodata->ime_composition[0]);
    videodata->ime_cursor = LOWORD(ImmGetCompositionStringW(himc, GCS_CURSORPOS, 0, 0));
    /*if (videodata->ime_cursor < SDL_arraysize(videodata->ime_composition) && videodata->ime_composition[videodata->ime_cursor] == 0x3000) {
        int i;
        for (i = videodata->ime_cursor + 1; i < length; ++i)
            videodata->ime_composition[i - 1] = videodata->ime_composition[i];

        --length;
    }*/
    videodata->ime_composition[length] = 0;
}

static void
IME_SendInputEvent(SDL_VideoData *videodata)
{
    char *s = 0;
    s = WIN_StringToUTF8(videodata->ime_composition);
    SDL_SendKeyboardText(s);
    SDL_free(s);

    videodata->ime_composition[0] = 0;
    videodata->ime_readingstring[0] = 0;
    videodata->ime_cursor = 0;
}

static void
IME_SendEditingEvent(SDL_VideoData *videodata)
{
    char *s = 0;
    /*WCHAR buffer[SDL_TEXTEDITINGEVENT_TEXT_SIZE];*/
    WCHAR buffer[512];
    const size_t size = SDL_arraysize(buffer);
    buffer[0] = 0;
    if (videodata->ime_readingstring[0]) {
        size_t len = SDL_min(SDL_wcslen(videodata->ime_composition), (size_t)videodata->ime_cursor);
        SDL_wcslcpy(buffer, videodata->ime_composition, len + 1);
        SDL_wcslcat(buffer, videodata->ime_readingstring, size);
        SDL_wcslcat(buffer, &videodata->ime_composition[len], size);
    }
    else {
        SDL_wcslcpy(buffer, videodata->ime_composition, size);
    }
    s = WIN_StringToUTF8(buffer);
    SDL_SendEditingText(s, videodata->ime_cursor + SDL_wcslen(videodata->ime_readingstring), 0);
    SDL_free(s);
}

SDL_bool
IME_HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM *lParam, SDL_VideoData *videodata)
{
    SDL_bool trap = SDL_FALSE;
    HIMC himc = 0;
    if (!videodata->ime_initialized || !videodata->ime_available || !videodata->ime_enabled)
        return SDL_FALSE;

    switch (msg) {
    case WM_INPUTLANGCHANGE:
        IME_InputLangChanged(videodata);
        break;
    //case WM_IME_SETCONTEXT:
    //    *lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
    //    break;
    case WM_IME_STARTCOMPOSITION:
        trap = SDL_TRUE;
        break;
    case WM_IME_COMPOSITION:
        trap = SDL_TRUE;
        himc = ImmGetContext(hwnd);
        if (*lParam & GCS_RESULTSTR) {
            IME_GetCompositionString(videodata, himc, GCS_RESULTSTR);
            IME_SendInputEvent(videodata);
        }
        if (*lParam & GCS_COMPSTR) {
            if (!videodata->ime_uiless)
                videodata->ime_readingstring[0] = 0;

            IME_GetCompositionString(videodata, himc, GCS_COMPSTR);
            IME_SendEditingEvent(videodata);
        }
        ImmReleaseContext(hwnd, himc);
        videodata->ime_himc=himc;
        break;
    case WM_IME_ENDCOMPOSITION:
        videodata->ime_composition[0] = 0;
        videodata->ime_readingstring[0] = 0;
        videodata->ime_cursor = 0;
        SDL_SendEditingText("", 0, 0);
        break;
    case WM_IME_NOTIFY:
        trap = SDL_FALSE;
        switch (wParam) {
        case IMN_SETCONVERSIONMODE:
        case IMN_SETOPENSTATUS:
            IME_UpdateInputLocale(videodata);
            break;
        default:
            break;
        }
        break;
    }
    return trap;
}

static void
IME_CloseCandidateList(SDL_VideoData *videodata)
{
    IME_HideCandidateList(videodata);
    videodata->ime_candcount = 0;
    SDL_memset(videodata->ime_candidates, 0, sizeof(videodata->ime_candidates));
}

#endif /* SDL_DISABLE_WINDOWS_IME */

#endif /* SDL_VIDEO_DRIVER_WINDOWS */

/* vi: set ts=4 sw=4 expandtab: */
