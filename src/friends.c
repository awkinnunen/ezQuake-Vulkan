/* OpenAI Codex, GPL-2.0-or-later. Friends menus and main-thread bridge.
 * Invitation secrets never enter cvars, console commands or console output.
 */
#include "quakedef.h"
#include "menu.h"
#include "common_draw.h"
#include "settings.h"
#include "settings_page.h"
#include "menu_local.h"
#include "server.h"
#include "utils.h"
#include "rulesets.h"
#include "friends.h"
#ifdef WITH_FRIENDS
#include "friends/transport.h"
#include "friends/windows.h"

static qbool private_game, joining, connected;
static char notice[192], last_message[192], pending_invitation[512];
static settings_page page, confirm_page;
static int confirmation, guest_index;
static int join_request;
static qbool staged_invitation;
static qbool developer_poll;
static double developer_poll_time;
static nf_status status;

/* Remote stufftext must not access private files/clipboard or host controls. */
static qbool LocalCommand(void) { return cbuf_current != &cbuf_svc; }
static void Notice(const char *s) { strlcpy(notice,s,sizeof(notice)); }
static void Refresh(void) { NF_Status(&status); }
static qbool HostingPossible(void) { return com_serveractive && sv.state==ss_active && maxclients.value>1; }
void Friends_HostFromArena(void) {
 char path[MAX_OSPATH];
 if(!HostingPossible()){Notice("Start a multiplayer game in Local Arena first.");return;}
 Refresh();
 if(private_game && status.mode==1 && status.ready){NF_Accept(1);Notice("Opening invitations...");return;}
 snprintf(path,sizeof(path),"%s/ezquake/friends.identity",com_basedir);
 if(NF_Host(path)){private_game=true;Notice("Opening your saved room...");}
 else Notice("Could not start the Friends connection worker.");
}
void Friends_Host(void){if(LocalCommand())Friends_HostFromArena();}
static void Close(void){if(!LocalCommand())return;NF_Accept(0);Notice("Invitations closed. Current guests may stay.");}
static void Copy(void){char link[512];if(!LocalCommand())return;if(NF_Invitation(link,sizeof(link))){Sys_CopyToClipboard(link);Notice("Invitation copied. It stays the same until Change invitation.");}else Notice("Wait for the room to become ready.");}
static void Rotate(void){if(!LocalCommand())return;confirmation=1;Settings_OnShow(&confirm_page);}
static void Back(void){confirmation=0;M_LeaveMenu(m_main);}
static void Cancel(void){confirmation=0;pending_invitation[0]=0;}
static void Join(void){
 if(!LocalCommand()||!NF_ValidateInvite(pending_invitation))return;
 Host_EndGame();
 join_request=NF_Join(pending_invitation);
 if(join_request){joining=true;Notice("Connecting to your friend's game...");}
 else Notice("Could not start the Friends connection worker.");
 pending_invitation[0]=0;confirmation=0;
}
static void Confirm(void){if(confirmation==1){NF_Rotate();Notice("Replacing the invitation...");confirmation=0;}else if(confirmation==2)Join();}
static void Paste(void){
 const char *s;
 if(!LocalCommand())return;
 s=ReadFromClipboard();
 if(!s||!NF_ValidateInvite(s)){Notice("Clipboard does not contain a valid ezQuake game invitation.");return;}
 strlcpy(pending_invitation,s,sizeof(pending_invitation));
 confirmation=2;Settings_OnShow(&confirm_page);
}
int Friends_ReceiveInvitation(const char *link){
 if(!NF_ValidateInvite(link))return 0;
 strlcpy(pending_invitation,link,sizeof(pending_invitation));staged_invitation=true;return 1;
}
static const char *LinkRegistration(void){return FriendsWindows_Registered(com_basedir)?"Enabled":"Disabled";}
static void RegisterLinks(qbool back){
 int enable;if(!LocalCommand())return;enable=!FriendsWindows_Registered(com_basedir);
 if(FriendsWindows_Register(com_basedir,enable))Notice(enable?"Invitation links now open this installation.":"Invitation link registration removed.");
 else Notice("Could not change invitation link registration.");
}
static client_t *Guest(void){
 int i,n=0;client_t *first=NULL;
 for(i=0;i<MAX_CLIENTS;++i)if(svs.clients[i].state>=cs_connected&&svs.clients[i].netchan.remote_address.type==NA_FRIENDS){
  if(!first)first=&svs.clients[i];if(n++==guest_index)return &svs.clients[i];
 }guest_index=0;return first;
}
static const char *GuestName(void){client_t *p=Guest();return p?p->name:"No guests";}
static void NextGuest(qbool back){guest_index=max(0,guest_index+(back?-1:1));Guest();}
static void RemoveGuest(void){client_t *p;if(!LocalCommand())return;p=Guest();if(p){NF_Drop(p->netchan.remote_address.friends_peer);SV_DropClient(p);Notice("Guest removed. Change invitation to prevent them rejoining.");}}
static setting entries[]={
 ADDSET_SEPARATOR("Friends / host this game"),
 ADDSET_ACTION("Local Arena - new game",MLocal_Open,"Choose a map, mode and bots. Select Friends under Who can join to open invitations when the game starts."),
 ADDSET_ACTION("Open / resume invitations",Friends_Host,"Allow friends into this local multiplayer game. Your invitation survives maps, restarts and updates. Direct Internet connectivity is required; some routers cannot connect."),
 ADDSET_ACTION("Copy invitation",Copy,"Copy the private reusable link to the clipboard. Send it to your friends. Keep the host game running while they join; the link works again when you reopen invitations."),
 ADDSET_ACTION("Close invitations",Close,"Stop admitting new guests. Existing players can stay, and the same invitation works after you reopen it."),
 ADDSET_ACTION("Change invitation",Rotate,"Replace the secret in your saved invitation. Old links stop admitting new guests. Existing guests stay. Confirmation required."),
 ADDSET_CUSTOM("Guest",GuestName,NextGuest,"Choose a connected guest to remove. Each friend has a separate connection and is never treated as the local player."),
 ADDSET_ACTION("Remove selected guest",RemoveGuest,"Disconnect this guest. To stop them rejoining, also change the invitation."),
 ADDSET_SEPARATOR("Friends / join a game"),
 ADDSET_ACTION("Paste invitation and join",Paste,"Read a game invitation from the clipboard. Shows a confirmation before leaving your current game. No additional program is needed."),
 ADDSET_CUSTOM("Windows links",LinkRegistration,RegisterLinks,"Register ezquake-vulkan:// links for this Windows user and game folder. Clicking a link opens a confirmation here, or starts this installation. Does not change qw://. Toggle again to remove this installation's registration."),
 ADDSET_ACTION("Back",Back,"Return to the previous menu without stopping the game."),
};
static setting confirm_entries[]={
 ADDSET_SEPARATOR("Confirm"),
 ADDSET_ACTION("Confirm",Confirm,"Carry out the action shown above."),
 ADDSET_ACTION("Cancel",Cancel,"Keep the current game and invitation."),
};
static const char *Policy(const setting *row){
 Refresh();
 if(row->actionfnc==Friends_Host&&!HostingPossible())return "Start a multiplayer game in Local Arena first.";
 if(row->actionfnc==Copy||row->actionfnc==Close||row->actionfnc==Rotate){if(!private_game||status.mode!=1||!status.ready)return "Open invitations and wait for the room to become ready.";}
 if(row->actionfnc==RemoveGuest||row->readfnc==GuestName){if(!private_game||!Guest())return "No connected Friends guests.";}
 return NULL;
}
void Friends_Open(void){if(!LocalCommand())return;confirmation=0;M_EnterMenu(m_friends);Settings_OnShow(&page);}
void Friends_Draw(void){
 Refresh();M_Unscale_Menu();
 UI_Print(16,16,confirmation==1?"Replace invitation? Old links stop working.":confirmation==2?"Join friend? Leave the current game?":"Friends / invitations",false);
 UI_Print(16,32,notice[0]?notice:status.message[0]?status.message:"Start a game in Local Arena, or paste an invitation.",false);
 UI_Print(16,48,va("%s | Guests: %d | %s",status.mode==1?"Hosting":status.mode==2?"Joining / connected":"Offline",status.peers,status.mode==1?(status.accepting?"Invitations open":"Invitations closed"):""),false);
 Settings_Draw(0,64,vid.width,vid.height-64,confirmation?&confirm_page:&page);
}
void Friends_Key(int key){if(key==K_ESCAPE||key==K_MOUSE2){if(confirmation)Cancel();else Back();return;}Settings_Key(confirmation?&confirm_page:&page,key,0);}
qbool Friends_Mouse(const mouse_state_t *ms){mouse_state_t m=*ms;if(ms->button_up==2){Friends_Key(K_ESCAPE);return true;}m.y-=64;m.y_old-=64;return Settings_Mouse_Event(confirmation?&confirm_page:&page,&m);}
void Friends_Frame(void){
 int i;
 if(developer_poll&&developer.value&&host_everything_loaded&&curtime>=developer_poll_time){developer_poll_time=curtime+0.25;Cbuf_AddText("exec control.cfg\n");}
 FriendsWindows_Poll();
 if(staged_invitation&&host_everything_loaded&&!Rulesets_RestrictIPC()){
  staged_invitation=false;M_EnterMenu(m_friends);confirmation=2;Settings_OnShow(&confirm_page);Notice("Invitation received. Confirm to join, or cancel to keep playing.");Con_Printf("Friends: Invitation received; waiting for your confirmation.\n");
 }
 Refresh();
 if(status.message[0]&&strcmp(last_message,status.message)){strlcpy(last_message,status.message,sizeof(last_message));Notice(status.message);Con_Printf("Friends: %s\n",status.message);}
 if(joining&&status.request==join_request&&status.client_peer){joining=false;connected=true;strlcpy(cls.servername,"friends",sizeof(cls.servername));M_LeaveMenus();CL_BeginServerConnect();}
 else if(joining&&status.request==join_request&&status.mode==0)joining=false;
 else if(connected&&!status.client_peer){connected=false;CL_Disconnect();Notice("Friends connection closed. Paste the invitation to reconnect.");}
 if(private_game&&com_serveractive)for(i=0;i<MAX_CLIENTS;++i){client_t *p=&svs.clients[i];if(p->state>=cs_preconnected&&p->netchan.remote_address.type==NA_FRIENDS&&!NF_Alive(p->netchan.remote_address.friends_peer))SV_DropClient(p);}
}
void Friends_ServerClosed(void){if(private_game){private_game=false;NF_Stop();}}
void Friends_Disconnected(void){if(joining||connected){joining=connected=false;NF_Stop();}}
qbool Friends_Private(void){return private_game;}
qbool Friends_Address(netadr_t *a){Refresh();if(!status.client_peer)return false;memset(a,0,sizeof(*a));a->type=NA_FRIENDS;a->friends_peer=status.client_peer;return true;}
qbool Friends_GetPacket(netsrc_t source){
 uint64_t peer;int n=NF_Receive(source==NS_SERVER,&peer,net_message.data,net_message.maxsize);
 if(!n)return false;memset(&net_from,0,sizeof(net_from));net_from.type=NA_FRIENDS;net_from.friends_peer=peer;net_message.cursize=n;return true;
}
void Friends_SendPacket(netsrc_t source,int length,void *data,netadr_t to){NF_Send(source==NS_SERVER,to.friends_peer,data,length);}
/* Explicit developer-only local file exchange lets integration tests use a
 * real engine without putting private URLs into process arguments or logs. */
static void Developer(void){
 FILE *f;char link[512];size_t n;int i,players=0;
 if(!developer.value||!LocalCommand())return;
 if(!strcmp(Cmd_Argv(1),"poll")){developer_poll=true;return;}
 if(!strcmp(Cmd_Argv(1),"host"))Friends_Host();
 else if(!strcmp(Cmd_Argv(1),"close"))Close();
 else if(!strcmp(Cmd_Argv(1),"rotate"))NF_Rotate();
 else if(!strcmp(Cmd_Argv(1),"export")&&NF_Invitation(link,sizeof(link))){f=fopen(Cmd_Argv(2),"wb");if(f){fwrite(link,1,strlen(link),f);fclose(f);}}
 else if(!strcmp(Cmd_Argv(1),"join")){f=fopen(Cmd_Argv(2),"rb");if(f){n=fread(link,1,sizeof(link)-1,f);fclose(f);link[n]=0;if(NF_ValidateInvite(link)){strlcpy(pending_invitation,link,sizeof(pending_invitation));Join();}}}
 else if(!strcmp(Cmd_Argv(1),"key"))Friends_Key(Key_StringToKeynum(Cmd_Argv(2)));
 for(i=0;i<MAX_CLIENTS;++i)if(svs.clients[i].state==cs_spawned&&!svs.clients[i].isBot)++players;
 Refresh();Con_Printf("FRIENDS_STATE mode=%d ready=%d accepting=%d peers=%d private=%d client=%d players=%d map=%s\n",status.mode,status.ready,status.accepting,status.peers,private_game,cls.state,players,Cvar_String("mapname"));
 Con_Printf("FRIENDS_UI menu=%d confirm=%d registered=%d staged=%d loaded=%d restricted=%d\n",m_state==m_friends,confirmation,FriendsWindows_Registered(com_basedir),staged_invitation,host_everything_loaded,Rulesets_RestrictIPC());
}
void Friends_Init(void){Settings_Page_Init(page,entries);Settings_Page_Init(confirm_page,confirm_entries);Settings_AddPolicy(Policy);FriendsWindows_Init(com_basedir);Cmd_AddCommand("menu_friends",Friends_Open);Cmd_AddCommand("friends_host",Friends_Host);Cmd_AddCommand("friends_close",Close);Cmd_AddCommand("friends_copy",Copy);Cmd_AddCommand("dev_friends",Developer);}
void Friends_Shutdown(void){FriendsWindows_Close();NF_Shutdown();Settings_Shutdown(&page);Settings_Shutdown(&confirm_page);}
#else
void Friends_Init(void){}
void Friends_Shutdown(void){}
void Friends_Frame(void){}
void Friends_Host(void){}
void Friends_HostFromArena(void){}
void Friends_ServerClosed(void){}
void Friends_Disconnected(void){}
qbool Friends_Private(void){return false;}
qbool Friends_Address(netadr_t *a){return false;}
qbool Friends_GetPacket(netsrc_t s){return false;}
void Friends_SendPacket(netsrc_t s,int n,void *d,netadr_t a){}
void Friends_Open(void){}
void Friends_Draw(void){}
void Friends_Key(int key){}
qbool Friends_Mouse(const mouse_state_t *ms){return false;}
#endif
