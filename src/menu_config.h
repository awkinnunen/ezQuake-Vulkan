/* OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZ_MENU_CONFIG_H
#define EZ_MENU_CONFIG_H
void MConfig_Init(void);
void MConfig_Shutdown(void);
void MConfig_Open(void);
void MConfig_Draw(int x, int y, int w, int h);
qbool MConfig_Key(int key);
qbool MConfig_Mouse(const mouse_state_t *ms);
#endif
