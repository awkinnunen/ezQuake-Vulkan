/* Config browser and keyboard preview, OpenAI Codex, 2026-09-13.
 * GPL-2.0-or-later; uses the existing ezQuake file list and drawing API.
 */
#include "quakedef.h"
#include "keys.h"
#include "Ctrl.h"
#include "EX_FileList.h"
#include "config_manager.h"
#include "cfg_bindings.h"
#include "menu_config.h"

typedef struct { const char *name; float x, y, w; } keycap_t;
static const keycap_t caps[] = {
	{"ESCAPE",0,0,1.4f},{"F1",2,0,1},{"F2",3,0,1},{"F3",4,0,1},{"F4",5,0,1},
	{"F5",6.5f,0,1},{"F6",7.5f,0,1},{"F7",8.5f,0,1},{"F8",9.5f,0,1},
	{"F9",11,0,1},{"F10",12,0,1},{"F11",13,0,1},{"F12",14,0,1},
	{"`",0,1,1},{"1",1,1,1},{"2",2,1,1},{"3",3,1,1},{"4",4,1,1},{"5",5,1,1},
	{"6",6,1,1},{"7",7,1,1},{"8",8,1,1},{"9",9,1,1},{"0",10,1,1},{"-",11,1,1},{"=",12,1,1},{"BACKSPACE",13,1,2},
	{"TAB",0,2,1.5f},{"q",1.5f,2,1},{"w",2.5f,2,1},{"e",3.5f,2,1},{"r",4.5f,2,1},{"t",5.5f,2,1},
	{"y",6.5f,2,1},{"u",7.5f,2,1},{"i",8.5f,2,1},{"o",9.5f,2,1},{"p",10.5f,2,1},{"[",11.5f,2,1},{"]",12.5f,2,1},{"\\",13.5f,2,1.5f},
	{"CAPSLOCK",0,3,1.75f},{"a",1.75f,3,1},{"s",2.75f,3,1},{"d",3.75f,3,1},{"f",4.75f,3,1},{"g",5.75f,3,1},
	{"h",6.75f,3,1},{"j",7.75f,3,1},{"k",8.75f,3,1},{"l",9.75f,3,1},{";",10.75f,3,1},{"'",11.75f,3,1},{"ENTER",12.75f,3,2.25f},
	{"LSHIFT",0,4,2.25f},{"z",2.25f,4,1},{"x",3.25f,4,1},{"c",4.25f,4,1},{"v",5.25f,4,1},{"b",6.25f,4,1},
	{"n",7.25f,4,1},{"m",8.25f,4,1},{",",9.25f,4,1},{".",10.25f,4,1},{"/",11.25f,4,1},{"RSHIFT",12.25f,4,2.75f},
	{"LCTRL",0,5,1.5f},{"LWIN",1.5f,5,1.5f},{"LALT",3,5,1.5f},{"SPACE",4.5f,5,6},{"RALT",10.5f,5,1.5f},{"MENU",12,5,1.5f},{"RCTRL",13.5f,5,1.5f},
	{"INS",15.5f,1,1},{"HOME",16.5f,1,1},{"PGUP",17.5f,1,1},
	{"DEL",15.5f,2,1},{"END",16.5f,2,1},{"PGDN",17.5f,2,1},
	{"UPARROW",16.5f,4,1},{"LEFTARROW",15.5f,5,1},{"DOWNARROW",16.5f,5,1},{"RIGHTARROW",17.5f,5,1},
	{"MOUSE1",0,6.3f,2},{"MOUSE2",2,6.3f,2},{"MOUSE3",4,6.3f,2},{"MOUSE4",6,6.3f,2},{"MOUSE5",8,6.3f,2},
	{"MWHEELUP",10,6.3f,2.5f},{"MWHEELDOWN",12.5f,6.3f,3},{"ISO",16.5f,6.3f,2}
};
static filelist_t files;
static cfg_bindings_t preview;
static char preview_path[MAX_OSPATH], notice[256];
static int selected_key = 'w', width, height, list_height, map_top;
static float unit, row_height;
static qbool readable;
static int detail_offset;
static void DeveloperCommand(void);

static int KeyNumber(const char *name, int physical)
{
	/* cfg_load starts in physical mode; named keys are layout independent. */
	return Key_StringToKeynumEx(name, physical != 0);
}

static void SetBinding(cfg_bindings_t *p, int key, const char *value)
{
#ifndef __APPLE__
	if (key == K_CTRL || key == K_ALT || key == K_SHIFT || key == K_WIN) {
		SetBinding(p, key + 1, value); SetBinding(p, key + 2, value); return;
	}
#endif
	if (key < 0 || key >= CFG_BINDING_KEYS) return;
	strlcpy(p->bindings[key], value, sizeof(p->bindings[key]));
	p->declared[key] = 1;
}

static const char *CurrentPath(void)
{
	filedesc_t *entry;
	if (files.need_refresh || files.error || files.num_entries < 1 || files.current_entry < 0 || files.current_entry >= files.num_entries) return NULL;
	entry = FL_GetCurrentEntry(&files);
	if (entry->is_directory || strcasecmp(COM_FileExtension(entry->name), "cfg")) return NULL;
	return entry->name;
}

static void ReadPreview(void)
{
	const char *path = CurrentPath();
	FILE *f;
	long length;
	char *data;
	if (!path) path = "";
	if (!strcmp(path, preview_path)) return;
	strlcpy(preview_path, path, sizeof(preview_path));
	memset(&preview, 0, sizeof(preview));
	readable = false;
	notice[0] = 0;
	if (!*path) return;
	f = fopen(path, "rb");
	if (!f) { strlcpy(notice, "Cannot read selected config. Press F5 to refresh.", sizeof(notice)); return; }
	if (fseek(f, 0, SEEK_END) || (length = ftell(f)) < 0 || length > 1024 * 1024 || fseek(f, 0, SEEK_SET)) {
		fclose(f); strlcpy(notice, "Config cannot be previewed (maximum 1 MiB).", sizeof(notice)); return;
	}
	data = Q_malloc(length + 1);
	if (fread(data, 1, length, f) != (size_t)length) {
		strlcpy(notice, "Could not read the complete config.", sizeof(notice));
	} else {
		data[length] = 0;
		CfgBindings_Parse(&preview, data, length, KeyNumber, SetBinding);
		readable = true;
	}
	fclose(f);
	Q_free(data);
}

void MConfig_Init(void)
{
	FL_Init(&files, ".");
	FL_SetDirUpOption(&files, false);
	FL_SetDirsOption(&files, false);
	FL_AddFileType(&files, 0, ".cfg");
	files.registered_types_only = true;
	if (IsDeveloperMode()) Cmd_AddCommand("dev_config_browser", DeveloperCommand);
}
void MConfig_Shutdown(void) { FL_Shutdown(&files); }
void MConfig_Open(void)
{
	char path[MAX_OSPATH];
	Cfg_GetConfigPath(path, sizeof(path), "");
	/* Keep a missing directory visible as an error instead of showing an old one. */
	if (!Sys_fullpath(files.current_dir, path, sizeof(files.current_dir))) files.current_dir[0] = 0;
	files.num_entries = 0;
	files.need_refresh = true;
	preview_path[0] = notice[0] = 0;
	memset(&preview, 0, sizeof(preview));
	detail_offset = 0;
	readable = false;
}

static void Text(int x, int y, int w, const char *text, float scale)
{
	/* Literal text: config strings must not be interpreted as colour escapes. */
	char line[256];
	int count = bound(0, (int)(w / (8 * scale)), (int)sizeof(line) - 1);
	strlcpy(line, text, count + 1);
	Draw_SString(x, y, line, scale, false);
}

void MConfig_Draw(int x, int y, int w, int h)
{
	int i, count = 0;
	float scale = w < 500 ? 0.75f : 1.0f;
	width = w; height = h;
	list_height = bound(80, h / 3, 160); /* FL_Draw requires at least 80 pixels. */
	row_height = max(4, (h - list_height - 78) / 7.3f);
	map_top = list_height + 16;
	unit = (w - 4) / 18.5f;
	Draw_AlphaFill(x, y, w, h, 0, 0.85f);
	FL_Draw(&files, x, y, w, list_height);
	ReadPreview();
	for (i = 0; i < CFG_BINDING_KEYS; i++) if (*preview.bindings[i]) count++;
	Text(x, y + list_height + 2, w, va("%d binds | Arrows: key | Shift+arrows: text", count), scale);
	for (i = 0; i < sizeof(caps) / sizeof(caps[0]); i++) {
		int key = KeyNumber(caps[i].name, 1);
		int kx = x + (int)(caps[i].x * unit), ky = y + map_top + (int)(caps[i].y * row_height);
		int kw = max(3, (int)(caps[i].w * unit) - 2), kh = (int)row_height - 2;
		qbool bound_key = key >= 0 && key < CFG_BINDING_KEYS && *preview.bindings[key];
		const char *label = caps[i].name;
		if (!strcmp(label,"BACKSPACE")) label="Back";
		else if (!strcmp(label,"CAPSLOCK")) label="Caps";
		else if (!strcmp(label,"UPARROW")) label="Up";
		else if (!strcmp(label,"DOWNARROW")) label="Dn";
		else if (!strcmp(label,"LEFTARROW")) label="Lt";
		else if (!strcmp(label,"RIGHTARROW")) label="Rt";
		Draw_Fill(kx, ky, kw, kh, key == selected_key ? 15 : 8);
		Draw_Fill(kx + 1, ky + 1, kw - 2, kh - 2, bound_key ? 104 : 3);
		{
			float ks = min(min(scale, (kw - 4) / (8.0f * strlen(label))), max(0.25f, (kh - 2) / 8.0f));
			Text(kx + 2, ky + (kh - (int)(8 * ks)) / 2, kw - 3, label, ks);
		}
	}
	detail_offset = min(detail_offset, (int)strlen(preview.bindings[selected_key]));
	Text(x, y + h - 56, w, va("%s: %s%s", Key_KeynumToString(selected_key), detail_offset ? "..." : "",
		*preview.bindings[selected_key] ? preview.bindings[selected_key] + detail_offset : preview.declared[selected_key] ? "Unbound" : "Not set in this file"), scale);
	Text(x, y + h - 43, w, "Preview only: includes, aliases and conditions are not evaluated.", scale);
	Text(x, y + h - 30, w, *notice ? notice : preview.warnings ? va("%u declaration(s) could not be previewed.", preview.warnings) : "Highlighted = bound. Layout uses engine key names (US positions).", scale);
	Draw_Fill(x, y + h - 16, min(w, 144), 15, readable ? 104 : 8);
	Text(x + 4, y + h - 13, 140, "Load config [Enter]", 0.85f);
	Text(x + 150, y + h - 13, w - 150, "F5: refresh   Esc: back", 0.85f);
}

qbool MConfig_Key(int key)
{
	if (key == K_ENTER) {
		const char *path = CurrentPath();
		if (path && readable && !strcmp(path, preview_path)) {
			if (Cfg_LoadConfigFile(path)) strlcpy(notice, "Config loaded.", sizeof(notice));
			else strlcpy(notice, "Config could not be loaded. Press F5 to refresh.", sizeof(notice));
		}
		return true;
	}
	if (key == K_F5) { files.need_refresh = true; preview_path[0] = 0; readable = false; memset(&preview, 0, sizeof(preview)); detail_offset = 0; return true; }
	if (key == K_LEFTARROW || key == K_RIGHTARROW) {
		int i;
		if (keydown[K_SHIFT]) {
			detail_offset = bound(0, detail_offset + (key == K_RIGHTARROW ? 8 : -8), (int)strlen(preview.bindings[selected_key]));
			return true;
		}
		detail_offset = 0;
		for (i = 0; i < CFG_BINDING_KEYS; i++) {
			selected_key = (selected_key + CFG_BINDING_KEYS + (key == K_RIGHTARROW ? 1 : -1)) % CFG_BINDING_KEYS;
			if (*preview.bindings[selected_key]) break;
		}
		return true;
	}
	/* This browser is restricted to its config directory; no file mutations. */
	if (key == K_UPARROW || key == K_DOWNARROW || key == K_PGUP || key == K_PGDN || key == K_HOME || key == K_END || key == K_MWHEELUP || key == K_MWHEELDOWN ||
		(key >= ' ' && key <= '~' && key != '/' && key != '\\' && !keydown[K_ALT] && !keydown[K_CTRL])) return FL_Key(&files, key);
	return true;
}

qbool MConfig_Mouse(const mouse_state_t *ms)
{
	int i;
	if (ms->x < 0 || ms->x >= width || ms->y < 0 || ms->y >= height) return false;
	if (ms->y < list_height) {
		mouse_state_t move = *ms;
		move.button_up = move.button_down = 0;
		FL_Mouse_Event(&files, &move);
		return true;
	}
	if (ms->button_up == 1) {
		if (ms->y >= height - 16 && ms->x < 144) return MConfig_Key(K_ENTER);
		for (i = 0; i < sizeof(caps) / sizeof(caps[0]); i++) {
			float kx = caps[i].x * unit, ky = map_top + caps[i].y * row_height;
			if (ms->x >= kx && ms->x < kx + caps[i].w * unit && ms->y >= ky && ms->y < ky + row_height) {
				int key = KeyNumber(caps[i].name, 1);
				if (key >= 0 && key < CFG_BINDING_KEYS) { selected_key = key; detail_offset = 0; }
				break;
			}
		}
	}
	return true;
}

/* Development-mode driver for the same actions used by the UI. */
static void DeveloperCommand(void)
{
	if (!strcmp(Cmd_Argv(1), "key")) MConfig_Key(KeyNumber(Cmd_Argv(2), 1));
	else if (!strcmp(Cmd_Argv(1), "click")) {
		int i;
		for (i = 0; i < sizeof(caps) / sizeof(caps[0]); i++) if (!strcmp(caps[i].name, Cmd_Argv(2))) {
			mouse_state_t mouse = {0};
			mouse.x = (caps[i].x + caps[i].w / 2) * unit;
			mouse.y = map_top + (caps[i].y + 0.5f) * row_height;
			mouse.button_up = 1;
			MConfig_Mouse(&mouse);
			Com_Printf("CFG_CLICK selected=%s\n", Key_KeynumToString(selected_key));
			break;
		}
	}
	else if (!strcmp(Cmd_Argv(1), "inspect")) {
		int key = KeyNumber(Cmd_Argv(2), 1);
		ReadPreview();
		Com_Printf("CFG_BROWSER files=%d readable=%d selected=%s\n", files.num_entries, readable, preview_path);
		if (key >= 0 && key < CFG_BINDING_KEYS)
			Com_Printf("CFG_PREVIEW %s = %s | active = %s\n", Cmd_Argv(2), preview.bindings[key], keybindings[key] ? keybindings[key] : "");
	}
	else Com_Printf("Usage: dev_config_browser key <key> | inspect <key>\n");
}
