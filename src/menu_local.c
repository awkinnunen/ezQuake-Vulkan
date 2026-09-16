/* Local multiplayer menu, OpenAI Codex, 2026-09-13.
 * GPL-2.0-or-later. Uses ezQuake's listen server and KTX bot commands.
 */
#include "quakedef.h"
#include "menu.h"
#include "common_draw.h"
#include "menu_local.h"
#include "friends.h"
#include "settings.h"
#include "settings_page.h"
#ifndef CLIENTONLY
#include "server.h"

extern void FS_EnumerateFiles(char *match, int (*func)(char *, int, void *), void *parm);
extern cvar_t sv_progtype;
extern int menuwidth;
#define LOCAL_MAPS 2048
#define LOCAL_ROWS 12
#define LOCAL_ROW_HEIGHT 9
#define MAP_ROWS 12
static char maps[LOCAL_MAPS][64];
static int map_count, map_index, cursor, bot_skill = 5, mode;
static qbool choosing_map, pending_start;
static qbool allow_friends, pending_friends;
#ifdef WITH_FRIENDS
static const char *ReadAccess(void){return allow_friends?"Friends (invitation)":"Only this computer";}
static void ToggleAccess(qbool back){allow_friends=!allow_friends;}
#endif
static int dm_mode = 3, pending_mode, pending_dm;
static char pending_map[64];
static const int mode_dm_defaults[] = {3, 3, 1, 5};
static settings_page arena_page, map_page;
static setting map_entries[LOCAL_MAPS+1];
static int initial_bots=1, pending_bots, pending_skill;
static void ChooseMap(void);
static char notice[160];
static const char *modes[] = { "Free for all", "Duel", "2 on 2", "Clan Arena" };
static const char *mode_commands[] = { "ffa", "1on1", "2on2", "carena" };

static qbool LocalServer(void)
{
    return com_serveractive && sv.state == ss_active && sv_progtype.value == 1
        && !strcmp(sv_progsname.string, "qwprogs") && *Info_ValueForKey(svs.info, "ktxver");
}

static qbool LocalConnected(void)
{
    return LocalServer() && cls.state == ca_active && !cls.demoplayback
        && cls.netchan.remote_address.type == NA_LOOPBACK;
}

static int AddMap(char *path, int size, void *unused)
{
    int i;
    size_t len;
    char name[64];
    if (strncmp(path, "maps/", 5)) return true;
    len = strlen(path + 5);
    if (len < 5 || len >= sizeof(name) || strcasecmp(path + strlen(path) - 4, ".bsp")) return true;
    memcpy(name, path + 5, len - 4);
    name[len - 4] = 0;
    /* Only map tokens, never paths or console command syntax. */
    for (i = 0; name[i]; ++i)
        if (!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z')
            || (name[i] >= '0' && name[i] <= '9') || name[i] == '_' || name[i] == '-')) return true;
    for (i = 0; i < map_count; ++i) if (!strcasecmp(maps[i], name)) return true;
    if (map_count < LOCAL_MAPS) strlcpy(maps[map_count++], name, sizeof(maps[0]));
    return true;
}

static int CompareMap(const void *a, const void *b) { return strcasecmp(a, b); }

static void RefreshMaps(void)
{
    char selected[64];
    int i;
    strlcpy(selected, map_count ? maps[map_index] : "aerowalk", sizeof(selected));
    map_count = 0;
    FS_EnumerateFiles("maps/*.bsp", AddMap, NULL);
    qsort(maps, map_count, sizeof(maps[0]), CompareMap);
    map_index = 0;
    for (i = 0; i < map_count; ++i) if (!strcasecmp(maps[i], selected)) map_index = i;
}

void MLocal_Open(void)
{
    RefreshMaps();
    choosing_map = false;
    notice[0] = 0;
    M_EnterMenu(m_local);
}

static void Start(void)
{
    vfsfile_t *f;
    if (!map_count) { strlcpy(notice, "No installed maps found. Press F5.", sizeof(notice)); return; }
    f = FS_OpenVFS(va("maps/%s.bsp", maps[map_index]), "rb", FS_ANY);
    if (!f) { strlcpy(notice, "Selected map is unavailable. Press F5.", sizeof(notice)); return; }
    VFS_CLOSE(f);
    /* Match the engine's native/QVM fallback, and fail before disconnecting. */
#ifdef _WIN32
    f = FS_OpenVFS("qwprogs.dll", "rb", FS_ANY);
#else
    f = FS_OpenVFS("qwprogs.so", "rb", FS_ANY);
#endif
    if (!f) f = FS_OpenVFS("qwprogs.qvm", "rb", FS_ANY);
    if (!f) { strlcpy(notice, "KTX is missing: install qwprogs first.", sizeof(notice)); return; }
    VFS_CLOSE(f);
    Host_EndGame();
    Cvar_Set(&sv_progsname, "qwprogs");
    Cvar_SetValue(&sv_progtype, 1);
    Cvar_SetValue(&maxclients, 16);
    Cvar_SetValue(&deathmatch, dm_mode);
    Cvar_SetValue(&coop, 0);
    Cvar_SetValue(&teamplay, 0);
    Cvar_SetByName("spectator", "0");
    if (!*Cvar_String("team")) Cvar_SetByName("team", "red");
    Cvar_SetByName("samelevel", "0");
    pending_start = true;
    pending_friends = allow_friends;
    pending_mode = mode;
    pending_dm = dm_mode;
    pending_bots = initial_bots; pending_skill = bot_skill;
    strlcpy(pending_map, maps[map_index], sizeof(pending_map));
    Cbuf_InsertText(va("map %s\n", maps[map_index]));
    strlcpy(notice, "Starting with selected rules...", sizeof(notice));
    M_LeaveMenus();
}

void MLocal_Connected(void)
{
    if (!pending_start) return;
    pending_start = false;
    if (!LocalConnected() || strcmp(sv.mapname, pending_map)) return;
    { char commands[1024]; int i;
      snprintf(commands,sizeof(commands),"cmd %s\ncmd dmm%d\n",mode_commands[pending_mode],pending_dm);
      for(i=0;i<pending_bots;++i)strlcat(commands,va("cmd botcmd addbot %d\n",pending_skill),sizeof(commands));
      Cbuf_InsertText(commands); }
    if(pending_friends)Friends_HostFromArena();
    strlcpy(notice, "Selected rules sent. Add bots when ready.", sizeof(notice));
}

static const char *ReadMap(void) { return map_count ? maps[map_index] : "No maps installed"; }
static const char *ReadMode(void) { return modes[mode]; }
static void ToggleMode(qbool back) { mode=(mode+(back?3:1))%4; dm_mode=mode_dm_defaults[mode]; }
static void Back(void) { M_LeaveMenu(m_main); }
static void SelectMap(void) { map_index=map_page.marked; choosing_map=false; }
static setting arena_entries[] = {
 ADDSET_SEPARATOR("Local Arena - new game"),
 ADDSET_ACTION("Choose map",ChooseMap,"Select an installed map. Selection does not change a running match."),
 ADDSET_CUSTOM("Game mode",ReadMode,ToggleMode,"Rules for the next local game. Current games are unchanged."),
 ADDSET_INTNUMBER("Deathmatch rules",dm_mode,1,5,1),
 ADDSET_INTNUMBER("Starting bots",initial_bots,0,15,1),
 ADDSET_INTNUMBER("Bot skill",bot_skill,1,20,1),

#ifdef WITH_FRIENDS
 ADDSET_CUSTOM("Who can join",ReadAccess,ToggleAccess,"Friends opens your saved invitation when the game starts. Only this computer uses the normal local game behavior."),
 ADDSET_ACTION("Friends / invitations",Friends_Open,"Copy a persistent link, join a friend, or manage invitations for the current game."),
#endif
 ADDSET_ACTION("Start new local game",Start,"Disconnect from the current game and start the selected map with KTX. Bots require navigation support for the map."),
 ADDSET_ACTION("Back to main menu",Back,"Return without changing the current game. Esc during play offers match and bot controls."),
};
static void ChooseMap(void) {
 int i;RefreshMaps();memset(map_entries,0,sizeof(map_entries));
 for(i=0;i<map_count;++i){map_entries[i].type=stt_action;map_entries[i].label=maps[i];map_entries[i].actionfnc=SelectMap;map_entries[i].description="Choose this map for the next local game. Nothing starts until Start new local game.";}
 if(!map_count){map_entries[0].type=stt_action;map_entries[0].label="No maps installed - F5 rescans";}
 map_page.count=max(1,map_count);map_page.marked=map_index;map_page.viewpoint=0;Settings_OnShow(&map_page);choosing_map=true;
}
void MLocal_Draw(void) {
 M_Unscale_Menu();UI_Print(16,16,va("Local Arena | Selected map: %.48s",ReadMap()),false);
 UI_Print(16,32,notice[0]?notice:"Setup for a new game. Esc during play: match and bot controls.",false);
 Settings_Draw(0,48,vid.width,vid.height-48,choosing_map?&map_page:&arena_page);
}
void MLocal_Key(int key) {
 if(key==K_ESCAPE||key==K_MOUSE2){if(choosing_map)choosing_map=false;else Back();return;}
 if(key==K_F5){if(choosing_map)ChooseMap();else RefreshMaps();return;}
 Settings_Key(choosing_map?&map_page:&arena_page,key,0);cursor=arena_page.marked;
}
qbool MLocal_Mouse(const mouse_state_t *ms) {
 mouse_state_t m=*ms;if(ms->button_up==2){MLocal_Key(K_ESCAPE);return true;}
 m.y-=48;m.y_old-=48;return Settings_Mouse_Event(choosing_map?&map_page:&arena_page,&m);
}

static void DeveloperCommand(void)
{
    int i, bots = 0;
    qbool local;
    char server_mode[64], server_status[64];
    extern void M_Main_Key(int key);
    if (!developer.value) return;
    /* Scripted UI tests wait with a nonempty buffer. Real mouse/key input
       arrives with an empty buffer and resets these guards naturally. */
    if (!strcmp(Cmd_Argv(1), "checkpoint")) {
        cbuf_main.runAwayLoop = cbuf_main.waitCount = 0;
        return;
    }
    if (!strcmp(Cmd_Argv(1), "mainkey") && m_state == m_main) M_Main_Key(Key_StringToKeynum(Cmd_Argv(2)));
    if (!strcmp(Cmd_Argv(1), "key") && m_state == m_local) MLocal_Key(Key_StringToKeynum(Cmd_Argv(2)));
    if (!strcmp(Cmd_Argv(1), "map") && m_state == m_local)
        for (i = 0; i < map_count; ++i) if (!strcmp(maps[i], Cmd_Argv(2))) map_index = i;
    if (!strcmp(Cmd_Argv(1), "start")) Start();
    if (!strcmp(Cmd_Argv(1), "bots")) initial_bots=bound(0,atoi(Cmd_Argv(2)),15);
    if (!strcmp(Cmd_Argv(1), "friends") && cbuf_current != &cbuf_svc) allow_friends=atoi(Cmd_Argv(2))!=0;
    if (LocalServer()) for (i = 0; i < MAX_CLIENTS; ++i)
        if (svs.clients[i].state != cs_free && svs.clients[i].isBot) ++bots;
    Con_Printf("LOCAL_MENU maps=%d selected=%s row=%d connected=%d bots=%d skill=%d server=%d notice=%s\n",
        map_count, map_count ? maps[map_index] : "none", cursor, LocalConnected(), bots, bot_skill, com_serveractive, notice);
    local = LocalServer();
    Con_Printf("LOCAL_DMM selected=%d live=%d\n", dm_mode, local ? (int)deathmatch.value : 0);
    /* Info_ValueForKey uses rotating static buffers: copy each result. */
    strlcpy(server_mode, local ? Info_ValueForKey(svs.info, "mode") : "none", sizeof(server_mode));
    strlcpy(server_status, local ? Info_ValueForKey(svs.info, "status") : "none", sizeof(server_status));
    Con_Printf("LOCAL_STATE menu=%d arena=%d mode=%s map=%s ca=%d status=%s\n", m_state, m_state == m_local,
        server_mode, local ? sv.mapname : "none", local ? (int)Cvar_Value("k_clan_arena") : 0, server_status);
    if (local && !strcmp(Cmd_Argv(1), "inspect")) {
        for (i = 0; i < MAX_CLIENTS; ++i) {
            client_t *bot = &svs.clients[i];
            if (bot->state != cs_free && bot->isBot)
                Con_Printf("LOCAL_BOT id=%d pos=%.0f,%.0f,%.0f\n", i,
                    bot->edict->v->origin[0], bot->edict->v->origin[1], bot->edict->v->origin[2]);
        }
    }
}

void MLocal_Init(void)
{
    Settings_Page_Init(arena_page,arena_entries);
    arena_entries[3].description="KTX deathmatch rule set for the next local game (1-5).";
    arena_entries[4].description="Number of bots to request after connecting. KTX checks map and mode support.";
    arena_entries[5].description="Skill of starting bots, from 1 to 20. Change live bots through the in-game menu.";
    map_entries[0]=arena_entries[1];Settings_Init(&map_page,map_entries,1,"arena_maps");
    Cmd_AddCommand("menu_local", MLocal_Open);
    Cmd_AddCommand("dev_local_menu", DeveloperCommand);
}
void MLocal_Shutdown(void) { Settings_Shutdown(&arena_page);Settings_Shutdown(&map_page); }
#else
void MLocal_Shutdown(void) {}
void MLocal_Connected(void) {}
void MLocal_Init(void) {}
void MLocal_Open(void) {}
void MLocal_Draw(void) {}
void MLocal_Key(int key) {}
qbool MLocal_Mouse(const mouse_state_t *ms) { return false; }
#endif
