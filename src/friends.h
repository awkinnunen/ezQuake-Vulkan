/* OpenAI Codex, GPL-2.0-or-later. Optional Friends game integration. */
#ifndef EZV_FRIENDS_H
#define EZV_FRIENDS_H
#include "keys.h"
void Friends_Init(void);
void Friends_Shutdown(void);
void Friends_Frame(void);
void Friends_Host(void);
void Friends_HostFromArena(void); /* only after a locally requested arena start */
void Friends_ServerClosed(void);
void Friends_Disconnected(void);
qbool Friends_Private(void);
qbool Friends_Address(netadr_t *address);
qbool Friends_GetPacket(netsrc_t source);
void Friends_SendPacket(netsrc_t source, int length, void *data, netadr_t to);
void Friends_Open(void);
void Friends_Draw(void);
void Friends_Key(int key);
qbool Friends_Mouse(const mouse_state_t *ms);
#endif
