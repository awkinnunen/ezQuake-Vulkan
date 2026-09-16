/* OpenAI Codex, GPL-2.0-or-later. Per-user protocol registration and profile IPC. */
#ifndef EZV_FRIENDS_PLATFORM_H
#define EZV_FRIENDS_PLATFORM_H
#ifdef __cplusplus
extern "C" {
#endif
int FriendsPlatform_Register(const char *basedir, int enable);
int FriendsPlatform_Registered(const char *basedir);
int FriendsPlatform_Forward(const char *basedir, const char *invitation);
void FriendsPlatform_Init(const char *basedir);
void FriendsPlatform_Poll(void);
void FriendsPlatform_Close(void);
/* Strict parser and private staging, never console command execution. */
int Friends_ReceiveInvitation(const char *invitation);
const char *FriendsPlatform_DefaultBase(void);
#ifdef __cplusplus
}
#endif
#endif
