/* MENU-UNIFY-001 / CFG-PRESETS-001, OpenAI Codex, 2026-09-15. GPL-2.0-or-later. */
#ifndef GRAPHICS_MENU_H
#define GRAPHICS_MENU_H
#include "settings.h"
void Graphics_Init(setting *original, int count);
void Graphics_Shutdown(void);
void Graphics_Draw(int x,int y,int w,int h);
qbool Graphics_Key(int key,wchar unichar);
qbool Graphics_Mouse(const mouse_state_t *ms);
void Graphics_OnShow(void);
void Graphics_CancelPreview(void);
void Graphics_Open(void);
void Graphics_OpenClarity(void);
void Graphics_VideoApplied(void);
int CV_NativeSettings(setting *out,int capacity);
const char *CV_NativeUnavailable(const setting *s);
qbool CV_RestartPending(void);
#endif
