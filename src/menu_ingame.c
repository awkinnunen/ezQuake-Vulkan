/*
Copyright (C) 2011 ezQuake team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
	\file

	\brief
	In-game menu

	\author
	johnnycz
**/

#include "quakedef.h"
#include "keys.h"
#include "menu.h"
#include "Ctrl.h"
#include "settings.h"
#include "settings_page.h"
#include "server.h"
#include "version.h"
#include "graphics_menu.h"
#include "menu_ingame.h"
#include "friends.h"

#define TOPMARGIN (6*LETTERWIDTH)

settings_page single_menu;
settings_page ingame_menu;
settings_page democtrl_menu;
settings_page botmatch_menu;
settings_page qtv_menu;

#define MENU_ALIAS(func,command,leavem) static void func(void) { Cbuf_AddText(command "\n"); if (leavem) M_LeaveMenus(); }

void MIng_MainMenu(void)		{ M_Menu_Main_f(); }
void MIng_Back(void)			{ M_LeaveMenus(); }

MENU_ALIAS(MIng_ServerBrowser,"menu_slist",false);
MENU_ALIAS(MIng_Options,"menu_options",false);
MENU_ALIAS(MIng_Join,"join",true);
MENU_ALIAS(MIng_Observe,"observe",true);
MENU_ALIAS(MIng_Disconnect,"disconnect",true);
MENU_ALIAS(MIng_Ready, "ready",true);
MENU_ALIAS(MIng_Break, "break",true);
MENU_ALIAS(MIng_SkillUp, "skillup",false);
MENU_ALIAS(MIng_SkillDown, "skilldown",false);
MENU_ALIAS(MIng_AddBot, "addbot",false);
MENU_ALIAS(MIng_RemoveBot, "removebot",false);
MENU_ALIAS(MIng_TeamBlue, "team blue;color 13",true);
MENU_ALIAS(MIng_TeamRed, "team red;color 4",true);
MENU_ALIAS(MDemoCtrl_DemoBrowser,"menu_demos",false);
MENU_ALIAS(MDemoCtrl_DemoControls, "demo_controls",true);
MENU_ALIAS(MSP_Load, "menu_load", false);
MENU_ALIAS(MSP_Save, "menu_save", false);
MENU_ALIAS(MQTV_Autotrack, "autotrack", true);
MENU_ALIAS(MQTV_Lastscores, "lastscores", true);
MENU_ALIAS(MQTV_Observers, "qtvusers", true);
MENU_ALIAS(MQTV_Reconnect, "qtvreconnect", true);

/* MENU-UNIFY-001, OpenAI Codex: shared local/online KTX controls. */
static settings_page ktx_bots_menu;
static qbool showing_bots;
static int new_bot_skill=5;
static char bot_notice[160];
static int BotCount(void){int i,n=0;for(i=0;i<MAX_CLIENTS;++i)if(cl.players[i].name[0]&&atoi(Info_ValueForKey(cl.players[i].userinfo,"*bot")))++n;return n;}
static const char *BotsUnavailable(void) {
 cmd_alias_t *a;
 if(cls.state!=ca_active||cls.demoplayback)return "Connect to a live KTX game to manage bots.";
 if(!*Info_ValueForKey(cl.serverinfo,"ktxver"))return "This server does not advertise KTX.";
 a=Cmd_FindAlias("botcmd");
 if(!a||!(a->flags&ALIAS_SERVER))return "This server has not advertised bot commands.";
 return NULL;
}
static void BotsBack(void){showing_bots=false;}
static void BotsOpen(void){if(BotsUnavailable())return;showing_bots=true;bot_notice[0]=0;Settings_OnShow(&ktx_bots_menu);}
static void BotRequest(const char *command){const char *why=BotsUnavailable();if(why){strlcpy(bot_notice,why,sizeof(bot_notice));return;}Cbuf_InsertText(va("cmd botcmd %s\n",command));strlcpy(bot_notice,"Request sent. Server checks permission and map support; see console.",sizeof(bot_notice));}
static void BotAdd(void){BotRequest(va("addbot %d",new_bot_skill));}
static void BotRemove(void){BotRequest("removebot");}
static void BotRemoveAll(void){BotRequest("removeall");}
static void BotSkill(void){BotRequest(va("skill %d",new_bot_skill));}
static setting ktx_bots_entries[]={
 ADDSET_SEPARATOR("In-game / Bots"),
 ADDSET_INTNUMBER("Bot skill",new_bot_skill,1,20,1),
 ADDSET_ACTION("Add bot",BotAdd,"Request one bot at the chosen skill. Works on local and online KTX servers that advertise bot support. Server permission and navigation support are required."),
 ADDSET_ACTION("Remove last bot",BotRemove,"Ask the current server to remove the last bot. The current map and match rules remain in place."),
 ADDSET_ACTION("Remove all bots",BotRemoveAll,"Ask the current server to remove every bot. The server checks permission."),
 ADDSET_ACTION("Set server's next bot skill",BotSkill,"Set the server's skill for the next bot added. Existing bots are unchanged; the server checks permission."),
 ADDSET_ACTION("Back to in-game menu",BotsBack,"Return to match controls."),
 ADDSET_ACTION("Return to game",MIng_Back,"Close the menu."),
};
static const char *IngamePolicy(const setting *row){
 if(row->actionfnc==BotsOpen||row->actionfnc==BotAdd||row->actionfnc==BotRemove||row->actionfnc==BotRemoveAll||row->actionfnc==BotSkill||row==&ktx_bots_entries[1])return BotsUnavailable();return NULL;
}
void Menu_Ingame_OnShow(void){showing_bots=false;}
static void DeveloperIngame(void){
 const char *a=Cmd_Argv(1),*why;
 if(!strcmp(a,"bots"))BotsOpen();else if(!strcmp(a,"add"))BotAdd();else if(!strcmp(a,"remove"))BotRemove();else if(!strcmp(a,"removeall"))BotRemoveAll();else if(!strcmp(a,"skill")){new_bot_skill=bound(1,atoi(Cmd_Argv(2)),20);BotSkill();}
 else if(!strcmp(a,"key"))M_Ingame_Key(Key_StringToKeynum(Cmd_Argv(2)));
 why=BotsUnavailable();Con_Printf("INGAME_STATE active=%d local=%d bots_page=%d available=%d skill=%d reason=%s\n",cls.state==ca_active,com_serveractive,showing_bots,!why,new_bot_skill,why?why:"server validates each request");
 Con_Printf("INGAME_BOTS count=%d\n",BotCount());
}

setting single_menu_entries[] = {
	ADDSET_SEPARATOR("In-game Menu"),
	ADDSET_ACTION("Load Game", MSP_Load, ""),
	ADDSET_ACTION("Save Game", MSP_Save, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Graphics and presets",Graphics_Open,"Adjust graphics and preview named presets without leaving the current server."),

#ifdef WITH_FRIENDS
	ADDSET_ACTION("Friends / invitations",Friends_Open,"Invite friends to a local game, close invitations, or join a saved link."),
#endif
	ADDSET_ACTION("Options", MIng_Options, "Controls, HUD, audio and other client settings."),
	ADDSET_ACTION("Main Menu", MIng_MainMenu, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Return To Game", MIng_Back, ""),
};

setting ingame_menu_entries[] = {
	ADDSET_SEPARATOR("In-game Menu"),
	ADDSET_ACTION("Bots",BotsOpen,"Manage bots on the current local or online KTX server. Availability follows server-advertised capabilities."),
	ADDSET_ACTION("Ready", MIng_Ready, "Mark yourself ready on the current server."),
	ADDSET_ACTION("Break", MIng_Break, "Request a match break. The server decides whether it is allowed."),
	ADDSET_ACTION("Join", MIng_Join, "Request to join the current game as a player."),
	ADDSET_ACTION("Observe", MIng_Observe, "Switch to observing the current game."),
	ADDSET_ACTION("Disconnect", MIng_Disconnect, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Server Browser", MIng_ServerBrowser, ""),
	ADDSET_ACTION("Graphics and presets",Graphics_Open,"Adjust graphics and preview named presets without leaving the current server."),

#ifdef WITH_FRIENDS
	ADDSET_ACTION("Friends / invitations",Friends_Open,"Invite friends to a local game, close invitations, or join a saved link."),
#endif
	ADDSET_ACTION("Options", MIng_Options, "Controls, HUD, audio and other client settings."),
	ADDSET_ACTION("Main Menu", MIng_MainMenu, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Return To Game", MIng_Back, ""),
};

setting democtrl_menu_entries[] = {
	ADDSET_SEPARATOR("Demo Control Menu"),
	ADDSET_ACTION("Toggle autotrack", MQTV_Autotrack, ""),
	ADDSET_ACTION("Demo controls", MDemoCtrl_DemoControls, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Disconnect", MIng_Disconnect, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Demo Browser", MDemoCtrl_DemoBrowser, ""),
	ADDSET_ACTION("Graphics and presets",Graphics_Open,"Adjust graphics and preview named presets without leaving the current server."),

#ifdef WITH_FRIENDS
	ADDSET_ACTION("Friends / invitations",Friends_Open,"Invite friends to a local game, close invitations, or join a saved link."),
#endif
	ADDSET_ACTION("Options", MIng_Options, "Controls, HUD, audio and other client settings."),
	ADDSET_ACTION("Main Menu", MIng_MainMenu, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Return To Demo", MIng_Back, ""),
};

setting qtv_menu_entries[] = {
	ADDSET_SEPARATOR("QuakeTV Menu"),
	ADDSET_ACTION("Toggle autotrack", MQTV_Autotrack, ""),
	ADDSET_ACTION("Last scores", MQTV_Lastscores, ""),
	ADDSET_ACTION("List observers", MQTV_Observers, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Reconnect", MQTV_Reconnect, ""),
	ADDSET_ACTION("Disconnect", MIng_Disconnect, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Graphics and presets",Graphics_Open,"Adjust graphics and preview named presets without leaving the current server."),

#ifdef WITH_FRIENDS
	ADDSET_ACTION("Friends / invitations",Friends_Open,"Invite friends to a local game, close invitations, or join a saved link."),
#endif
	ADDSET_ACTION("Options", MIng_Options, "Controls, HUD, audio and other client settings."),
	ADDSET_ACTION("Main Menu", MIng_MainMenu, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Return To Game", MIng_Back, ""),
};

setting botmatch_menu_entries[] = {
	ADDSET_SEPARATOR("Botmatch Menu"),
	ADDSET_ACTION("Ready", MIng_Ready, "Mark yourself ready on the current server."),
	ADDSET_ACTION("Break", MIng_Break, "Request a match break. The server decides whether it is allowed."),
	ADDSET_ACTION("Team Blue", MIng_TeamBlue, ""),
	ADDSET_ACTION("Team Red", MIng_TeamRed, ""),
	ADDSET_ACTION("Add Bot", MIng_AddBot, ""),
	ADDSET_ACTION("Remove Bot", MIng_RemoveBot, ""),
	ADDSET_ACTION("Increase Bot Skill", MIng_SkillUp, ""),
	ADDSET_ACTION("Decrease Bot Skill", MIng_SkillDown, ""),
	ADDSET_ACTION("Disconnect", MIng_Disconnect, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Graphics and presets",Graphics_Open,"Adjust graphics and preview named presets without leaving the current server."),

#ifdef WITH_FRIENDS
	ADDSET_ACTION("Friends / invitations",Friends_Open,"Invite friends to a local game, close invitations, or join a saved link."),
#endif
	ADDSET_ACTION("Options", MIng_Options, "Controls, HUD, audio and other client settings."),
	ADDSET_ACTION("Main Menu", MIng_MainMenu, ""),
	ADDSET_BLANK(),
	ADDSET_ACTION("Return To Game", MIng_Back, ""),
};

#define DEMOPLAYBACK() (cls.demoplayback && cls.mvdplayback != QTV_PLAYBACK)
#define BOTMATCH() (!strcmp(cls.gamedirfile, "fbca"))
#ifndef CLIENTONLY
#define SINGLEPLAYER() (com_serveractive && cls.state == ca_active && !cl.deathmatch && maxclients.value == 1)
#endif
#define QTVPLAYBACK() (cls.mvdplayback == QTV_PLAYBACK)

static settings_page *M_Ingame_Current(void) {
	if(showing_bots && cls.state==ca_active && !cls.demoplayback)return &ktx_bots_menu;
	showing_bots=false;
	if (DEMOPLAYBACK()) {
		return &democtrl_menu;
	}
	else if (BOTMATCH()) {
		return &botmatch_menu;
	}
#ifndef CLIENTONLY
	else if (SINGLEPLAYER()) {
		return &single_menu;
	}
#endif
	else if (QTVPLAYBACK()) {
		return &qtv_menu;
	}
	else {
		return &ingame_menu;
	}
}

void M_Ingame_Draw(void) {
	char version[VERSION_MAX_LEN] = { 0 };
	qbool outdated;

	M_Unscale_Menu();
	if(showing_bots)UI_Print(16,16,bot_notice[0]?bot_notice:"Bots on the current server | Esc: match controls",false);
	Settings_Draw(0, TOPMARGIN, vid.width, vid.height - TOPMARGIN, M_Ingame_Current());

	outdated = VersionCheck_GetLatest(version);
	if (outdated)
	{
		char message[4096] = { 0 };
		snprintf(message, sizeof(message), "Outdated client %s, latest version is %s", VERSION_NUMBER, version);
		UI_Print_Center(0, 32, vid.width, message, 0);
	}
}

void M_Ingame_Key(int key) {
	if(showing_bots&&(key==K_ESCAPE||key==K_MOUSE2)){showing_bots=false;return;}
	if (Settings_Key(M_Ingame_Current(), key, 0)) return;

	switch (key) {
	case K_MOUSE2:
	case K_ESCAPE: M_LeaveMenus(); break;
	}
}

qbool Menu_Ingame_Mouse_Event(const mouse_state_t *ms) {
	mouse_state_t m = *ms;
	if(ms->button_up==2){M_Ingame_Key(K_ESCAPE);return true;}
	m.y -= TOPMARGIN; m.y_old -= TOPMARGIN;
	return Settings_Mouse_Event(M_Ingame_Current(), &m);
}

void Menu_Ingame_Init(void)
{
	Settings_Page_Init(single_menu, single_menu_entries);
	Settings_Page_SetMinit(single_menu);
	Settings_Page_Init(ingame_menu, ingame_menu_entries);
	Settings_Page_Init(ktx_bots_menu,ktx_bots_entries);
	ktx_bots_entries[1].description="Skill for new bots, 1-20. Add bot sends this value explicitly. Existing bots are unchanged.";
	Settings_AddPolicy(IngamePolicy);
	if(IsDeveloperMode())Cmd_AddCommand("dev_ingame",DeveloperIngame);
	Settings_Page_Init(democtrl_menu, democtrl_menu_entries);
	Settings_Page_SetMinit(democtrl_menu);
	Settings_Page_Init(botmatch_menu, botmatch_menu_entries);
	Settings_Page_SetMinit(botmatch_menu);
	Settings_Page_Init(qtv_menu, qtv_menu_entries);
	Settings_Page_SetMinit(qtv_menu);
}

void Menu_Ingame_Shutdown(void)
{
	Settings_Shutdown(&single_menu);
	Settings_Shutdown(&ingame_menu);
	Settings_Shutdown(&ktx_bots_menu);
	Settings_Shutdown(&democtrl_menu);
	Settings_Shutdown(&botmatch_menu);
	Settings_Shutdown(&qtv_menu);
}
