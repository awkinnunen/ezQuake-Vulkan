/* Friends transport, OpenAI Codex, GPL-2.0-or-later. */
#ifndef EZV_FRIENDS_TRANSPORT_H
#define EZV_FRIENDS_TRANSPORT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int mode; /* 0 stopped, 1 host, 2 guest */
    int ready, accepting, peers;
    int request; /* start request generation; excludes a previous asynchronous join */
    uint64_t client_peer;
    char message[192];
} nf_status;
int NF_Host(const char *identity_path);
int NF_Join(const char *invitation);
int NF_ValidateInvite(const char *invitation);
void NF_Stop(void);
void NF_Shutdown(void);
void NF_Accept(int enabled);
void NF_Rotate(void);
void NF_Drop(uint64_t peer);
int NF_Alive(uint64_t peer);
void NF_Status(nf_status *out);
int NF_Invitation(char *out, int capacity);
int NF_Receive(int server, uint64_t *peer, void *data, int capacity);
void NF_Send(int server, uint64_t peer, const void *data, int size);
#ifdef __cplusplus
}
#endif
#endif
