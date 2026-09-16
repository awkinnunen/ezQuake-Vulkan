/* OpenAI Codex, GPL-2.0-or-later. Per-user protocol registration and profile IPC. */
#ifndef EZV_FRIENDS_WINDOWS_H
#define EZV_FRIENDS_WINDOWS_H
int FriendsWindows_Register(const char *basedir, int enable);
int FriendsWindows_Registered(const char *basedir);
int FriendsWindows_Forward(const char *basedir, const char *invitation);
void FriendsWindows_Init(const char *basedir);
void FriendsWindows_Poll(void);
void FriendsWindows_Close(void);
/* Strict parser and private staging, never console command execution. */
int Friends_ReceiveInvitation(const char *invitation);
#endif
