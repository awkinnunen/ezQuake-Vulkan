/* Local multiplayer menu, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZQUAKE_MENU_LOCAL_H
#define EZQUAKE_MENU_LOCAL_H
void MLocal_Init(void);
void MLocal_Shutdown(void);
void MLocal_Connected(void);
void MLocal_Open(void);
void MLocal_Draw(void);
void MLocal_Key(int key);
qbool MLocal_Mouse(const mouse_state_t *ms);
#endif
