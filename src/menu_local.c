/* Local multiplayer menu, OpenAI Codex, 2026-09-13.
 * GPL-2.0-or-later. Uses ezQuake's listen server and KTX bot commands.
 */
#include "quakedef.h"
#include "menu.h"
#include "common_draw.h"
#include "menu_local.h"
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
static int dm_mode = 3, pending_mode, pending_dm;
static char pending_map[64];
static const int mode_dm_defaults[] = {3, 3, 1, 5};
static menu_window_t window;
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
    pending_mode = mode;
    pending_dm = dm_mode;
    strlcpy(pending_map, maps[map_index], sizeof(pending_map));
    Cbuf_InsertText(va("map %s\n", maps[map_index]));
    strlcpy(notice, "Starting with selected rules...", sizeof(notice));
}

void MLocal_Connected(void)
{
    if (!pending_start) return;
    pending_start = false;
    if (!LocalConnected() || strcmp(sv.mapname, pending_map)) return;
    Cbuf_InsertText(va("cmd %s\ncmd dmm%d\n", mode_commands[pending_mode], pending_dm));
    strlcpy(notice, "Selected rules sent. Add bots when ready.", sizeof(notice));
}

static void Activate(void)
{
    if (cursor == 0) { choosing_map = true; return; }
    if (cursor == 1) { Start(); return; }
    if (cursor == 3 && !LocalConnected()) {
        strlcpy(notice, "Deathmatch applies at next local start.", sizeof(notice)); return;
    }
    if (cursor == 4) { bot_skill = bot_skill % 20 + 1; return; }
    if (cursor == 11) { M_LeaveMenu(m_main); return; }
    if (cursor == 10) {
        if (LocalServer()) {
            Host_EndGame();
            strlcpy(notice, "Local server stopped.", sizeof(notice));
        } else strlcpy(notice, "No local KTX server to stop.", sizeof(notice));
        return;
    }
    if (!LocalConnected()) {
        strlcpy(notice, "Start a local game and wait for connection.", sizeof(notice));
        return;
    }
    switch (cursor) {
    case 2: Cbuf_InsertText(va("cmd %s\ncmd dmm%d\n", mode_commands[mode], dm_mode)); break;
    case 3: Cbuf_InsertText(va("cmd dmm%d\n", dm_mode)); break;
    case 5: Cbuf_InsertText(va("cmd botcmd addbot %d\n", bot_skill)); break;
    case 6: Cbuf_InsertText("cmd botcmd removebot\n"); break;
    case 7: Cbuf_InsertText("cmd botcmd removeall\n"); break;
    case 8: Cbuf_InsertText("cmd ready\n"); M_LeaveMenus(); break;
    case 9: M_LeaveMenus(); break;
    }
    strlcpy(notice, "Request sent. KTX reports results in console.", sizeof(notice));
}

void MLocal_Draw(void)
{
    char text[96];
    int i, first, split, bots = 0;
    M_PrintWhite(80, 8, choosing_map ? "CHOOSE MAP" : "LOCAL ARENA");
    window.x = (menuwidth - 320) / 2 + 24;
    window.y = m_yofs + 40;
    window.w = 280;
    if (choosing_map) {
        first = (map_index / MAP_ROWS) * MAP_ROWS;
        window.h = min(MAP_ROWS, map_count - first) * 10;
        for (i = first; i < min(first + MAP_ROWS, map_count); ++i)
            M_Print(40, 40 + (i - first) * 10, maps[i]);
        if (map_count) M_DrawCharacter(24, 40 + (map_index - first) * 10, FLASHINGARROW());
        M_PrintWhite(24, 24, va("%d maps, %d/%d", map_count, map_count ? map_index + 1 : 0, map_count));
        M_PrintWhite(24, 174, "Enter: select  Esc: back  F5: scan");
        M_PrintWhite(24, 184, "PgUp/PgDn: page  Letter: jump");
        return;
    }
    if (LocalServer()) for (i = 0; i < MAX_CLIENTS; ++i)
        if (svs.clients[i].state != cs_free && svs.clients[i].isBot) ++bots;
    if (LocalConnected()) {
        snprintf(text, sizeof(text), "%.12s  %.10s  Bots: %d", sv.mapname, Info_ValueForKey(svs.info, "mode"), bots);
        if (!strncmp(notice, "Starting...", 11)) strlcpy(notice, "Connected. Apply a mode, then add bots.", sizeof(notice));
    } else strlcpy(text, LocalServer() ? "Connecting to local server..." : "No local KTX game", sizeof(text));
    M_PrintWhite(24, 24, text);
    window.h = LOCAL_ROWS * LOCAL_ROW_HEIGHT;
    for (i = 0; i < LOCAL_ROWS; ++i) {
        switch (i) {
        case 0: snprintf(text, sizeof(text), "Map: %.27s", map_count ? maps[map_index] : "(none)"); break;
        case 1: strlcpy(text, com_serveractive ? "Restart on selected map" : "Start local server", sizeof(text)); break;
        case 2: snprintf(text, sizeof(text), "Apply mode: %s", modes[mode]); break;
        case 3:
            if (LocalServer()) snprintf(text, sizeof(text), "Deathmatch: %d (live %d)", dm_mode, (int)deathmatch.value);
            else snprintf(text, sizeof(text), "Deathmatch: %d", dm_mode);
            break;
        case 4: snprintf(text, sizeof(text), "New bot skill: %d / 20", bot_skill); break;
        case 5: strlcpy(text, "Add bot", sizeof(text)); break;
        case 6: strlcpy(text, "Remove last bot", sizeof(text)); break;
        case 7: strlcpy(text, "Remove all bots", sizeof(text)); break;
        case 8: strlcpy(text, "Ready / start match", sizeof(text)); break;
        case 9: strlcpy(text, "Return to game", sizeof(text)); break;
        case 10: strlcpy(text, "Stop local server", sizeof(text)); break;
        default: strlcpy(text, "Back to main menu", sizeof(text)); break;
        }
        M_Print(40, 40 + i * LOCAL_ROW_HEIGHT, text);
    }
    M_DrawCharacter(24, 40 + cursor * LOCAL_ROW_HEIGHT, FLASHINGARROW());
    M_PrintWhite(24, 158, "Left/right: edit  Enter: apply");
    M_PrintWhite(24, 168, "Bot navigation depends on the map.");
    /* Two bounded lines fit the original 320x200 menu canvas. */
    split = min(35, strlen(notice));
    if (strlen(notice) > 35) {
        while (split > 0 && notice[split] != ' ') --split;
        if (!split) split = 35;
    }
    snprintf(text, sizeof(text), "%.*s", split, notice);
    M_PrintWhite(24, 180, text);
    if (strlen(notice) > split) {
        if (notice[split] == ' ') ++split;
        snprintf(text, sizeof(text), "%.35s", notice + split);
        M_PrintWhite(24, 190, text);
    }
}

void MLocal_Key(int key)
{
    int i, direction = key == K_LEFTARROW ? -1 : 1;
    if (key == K_ESCAPE || key == K_MOUSE2) {
        if (choosing_map) choosing_map = false;
        else M_LeaveMenu(m_main);
        return;
    }
    if (key == K_F5) { RefreshMaps(); return; }
    if (choosing_map) {
        if (!map_count) return;
        switch (key) {
        case K_UPARROW: case K_MWHEELUP: map_index = (map_index + map_count - 1) % map_count; break;
        case K_DOWNARROW: case K_MWHEELDOWN: map_index = (map_index + 1) % map_count; break;
        case K_PGUP: map_index = max(0, map_index - MAP_ROWS); break;
        case K_PGDN: map_index = min(map_count - 1, map_index + MAP_ROWS); break;
        case K_HOME: map_index = 0; break;
        case K_END: map_index = map_count - 1; break;
        case K_ENTER: case K_MOUSE1: choosing_map = false; break;
        default:
            for (i = 1; i <= map_count; ++i) {
                int index = (map_index + i) % map_count;
                if (tolower((unsigned char)maps[index][0]) == key) { map_index = index; break; }
            }
        }
        return;
    }
    switch (key) {
    case K_UPARROW: case K_MWHEELUP: cursor = (cursor + LOCAL_ROWS - 1) % LOCAL_ROWS; break;
    case K_DOWNARROW: case K_MWHEELDOWN: cursor = (cursor + 1) % LOCAL_ROWS; break;
    case K_HOME: cursor = 0; break;
    case K_END: cursor = LOCAL_ROWS - 1; break;
    case K_LEFTARROW: case K_RIGHTARROW:
        if (cursor == 2) {
            mode = (mode + 4 + direction) % 4;
            dm_mode = mode_dm_defaults[mode];
        }
        if (cursor == 3) dm_mode = (dm_mode - 1 + 5 + direction) % 5 + 1;
        if (cursor == 4) bot_skill = bound(1, bot_skill + direction, 20);
        break;
    case K_ENTER: case K_MOUSE1: Activate(); break;
    }
}

qbool MLocal_Mouse(const mouse_state_t *ms)
{
    int first = (map_index / MAP_ROWS) * MAP_ROWS, selected = map_index - first;
    if (choosing_map && map_count) {
        M_Mouse_Select(&window, ms, min(MAP_ROWS, map_count - first), &selected);
        map_index = first + selected;
    } else if (!choosing_map) M_Mouse_Select(&window, ms, LOCAL_ROWS, &cursor);
    if (ms->button_up == 1) MLocal_Key(K_MOUSE1);
    if (ms->button_up == 2) MLocal_Key(K_MOUSE2);
    return true;
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
    Cmd_AddCommand("menu_local", MLocal_Open);
    Cmd_AddCommand("dev_local_menu", DeveloperCommand);
}
#else
void MLocal_Connected(void) {}
void MLocal_Init(void) {}
void MLocal_Open(void) {}
void MLocal_Draw(void) {}
void MLocal_Key(int key) {}
qbool MLocal_Mouse(const mouse_state_t *ms) { return false; }
#endif
