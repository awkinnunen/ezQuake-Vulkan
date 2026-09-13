/* Competitive Visuals, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#include "quakedef.h"
#include "gl_model.h"
#include "tr_types.h"
#include "r_local.h"
extern void M_Print_GetPoint(int cx, int cy, int *rx, int *ry, const char *str, qbool red);
#include "rulesets.h"
#include "competitive_visuals.h"
#ifndef CLIENTONLY
#include "server.h"
#endif
#include <ctype.h>
#include <math.h>

#define CV_VAR(id,label,def,lo,hi,step,page,help) {"r_cv_" #id, def},
static cvar_t vars[] = { CV_OPTIONS(CV_VAR) };
#undef CV_VAR
typedef struct { const char *label, *def, *help; float lo, hi, step; int page; } cv_option_t;
#define CV_OPT(id,label,def,lo,hi,step,page,help) {label,def,help,lo,hi,step,page},
static const cv_option_t opts[] = { CV_OPTIONS(CV_OPT) };
#undef CV_OPT
typedef struct { const char *name, *label, *help; float lo, hi, step; int page; } cv_existing_t;
static const cv_existing_t existing[] = {
 {"gl_outline","Outline mode","0 off 1 models 2 world 3 both",0,3,1,3},
 {"gl_outline_scale_model","Model edge width","Subject to the active ruleset",0,1,.05,3},
 {"gl_outline_world_depth_threshold","Edge depth threshold","World depth discontinuity threshold",1,16,.5,3},
 {"gl_outline_use_player_color","Player edge colours","Use the player shirt colour",0,1,1,3},
 {"r_shadows","Model shadows","Projected shadows; fixed opacity",0,1,1,4},
 {"gl_detail","Detail overlay","Additional world detail texture",0,1,1,4},
 {"gl_caustics","Water caustics","Animated underwater decoration",0,1,1,4},
 {"vid_framebuffer_multisample","MSAA samples","Smooth geometry edges. Hardware may limit samples. Use Restart video or F5 to apply.",0,8,2,5},
 {"vid_framebuffer_fxaa","FXAA preset","Smooth scene edges after rendering. Zero disables FXAA; HUD stays sharp.",0,17,1,5},
 // Retained in the profile allowlist for compatibility; already in Graphics.
 {"r_rocketTrail","Rocket trail","Use Graphics > Projectiles",0,9,1,-1},
 {"r_grenadeTrail","Grenade trail","Use Graphics > Projectiles",0,3,1,-1},
 {"r_explosionType","Explosion style","Use Graphics > Projectiles",0,11,1,-1},
 {"cl_muzzleflash","Muzzle flashes","Use Graphics > Player & Weapon Model",0,2,1,-1},
 {"r_drawflame","Torch flames","Use Graphics > Environment",0,1,1,-1},
 {"gl_texturemode","Texture filtering","Nearest preserves pixels; trilinear smooths textures and mip transitions.",0,0,0,5}
};
#define CV_EXT_COUNT (sizeof(existing)/sizeof(existing[0]))
#define CV_TOTAL (CV_COUNT + CV_EXT_COUNT)
#define CV_RESTART_ROW CV_TOTAL
static qbool videoApplied;
static int appliedMSAA, actualMSAA;
void CV_VideoApplied(int requested, int actual) { videoApplied=true; appliedMSAA=requested; actualMSAA=actual; }
static qbool RestartPending(void) {
 cvar_t *v=Cvar_Find("vid_framebuffer_multisample");
 return R_UseVulkan() && videoApplied && v && v->integer!=appliedMSAA;
}
static const char *pages[] = {"Profiles", "World", "Lighting", "Silhouettes", "Effects", "Image"};
static char notice[160], profile[32] = "competitive", saved[CV_TOTAL][64];
static qbool comparing, editing, general;
static int page, cursor, indices[CV_TOTAL+16], depths[CV_TOTAL+16], rows;
static menu_window_t window;
extern cvar_t scr_scaleMenu;
extern int menuwidth, menuheight, m_yofs;
static int dragging = -1;
#define CV_SLIDER_X 192
#define CV_SLIDER_TRAVEL 40
#define CV_VISIBLE_ROWS 8
#define EXT(n) (CV_COUNT+(n))
static const char *groups[]={"World readability","Surface colours","Light response","Cel shading",
 "Player rim","Outline mode","World edges","Model edges","Edge colours",
 "Bloom","Ambient shading","Surface decoration","Scene image","Antialiasing / filtering"};
typedef struct { int index, page, group, depth; } cv_row_t;
static const cv_row_t layout[]={
 {CV_enable,1,0,0},{CV_detail,1,0,1},{CV_contrast,1,0,1},{CV_pattern,1,0,1},{CV_distance,1,0,1},{CV_saturation,1,0,1},
 {CV_tint,1,1,0},{CV_floor,1,1,1},{CV_wall,1,1,1},
 {CV_light,2,2,0},{CV_lightmix,2,2,1},{CV_worldlight,2,2,1},{CV_modellight,2,2,1},
 {CV_shadow,2,2,1},{CV_midtone,2,2,1},{CV_highlight,2,2,1},{CV_warmth,2,2,1},
 {CV_bands,2,3,1},{CV_softness,2,3,1},
 {CV_rim,3,4,0},{CV_rimwidth,3,4,1},{CV_rimup,3,4,1},{CV_rimteam,3,4,1},
 {EXT(0),3,5,0},
 {CV_edgestrength,3,6,1},{CV_edgewidth,3,6,1},{EXT(2),3,6,1},
 {EXT(1),3,7,1},{CV_modelopacity,3,7,1},{EXT(3),3,7,1},
 {CV_edgepalette,3,8,0},{CV_worldedge,3,8,1},{CV_teamedge,3,8,1},{CV_enemyedge,3,8,1},
 {CV_bloom,4,9,0},{CV_bloomthreshold,4,9,1},{CV_bloomradius,4,9,1},
 {CV_ao,4,10,0},{CV_aoradius,4,10,1},{EXT(4),4,10,0},
 {EXT(5),4,11,0},{EXT(6),4,11,0},
 {CV_exposure,5,12,0},{CV_tonemap,5,12,0},{CV_sharpen,5,12,0},
 {EXT(7),5,13,0},{EXT(8),5,13,0},{EXT(14),5,13,0},{CV_RESTART_ROW,5,13,0}
};
static cvar_t *Var(int index);
static float ExistingValue(const char *name) { cvar_t *v=Cvar_Find(name); return v?v->value:0; }
static const char *Unavailable(int index) {
 int outline=(int)ExistingValue("gl_outline");
 qbool world=(outline&2)!=0, model=(outline&1)!=0;
 model_t playerModel={0};
 playerModel.modhint=MOD_PLAYER;
 if(index==CV_RESTART_ROW) return RestartPending()?NULL:"No video restart is pending.";
 if(index<CV_COUNT && !R_UseVulkan()) return "Requires the Vulkan renderer.";
 if(index<CV_bloom && index!=CV_enable && !CV_Active()) return "Enable Competitive Visuals first.";
 if((index>=CV_rim && index<=CV_rimteam) && RuleSets_DisallowModelOutline(&playerModel)) return "Player styling blocked by ruleset.";
 if((index==CV_floor||index==CV_wall) && CV_Value(CV_tint)<=0) return "Increase Surface tint first.";
 if((index==CV_pattern||index==CV_distance) && CV_Value(CV_detail)>=.999 && fabsf(CV_Value(CV_contrast)-1)<=.001)
  return "Reduce Fine detail or contrast.";
 if(index>=CV_lightmix && index<=CV_warmth) {
  if(CV_Value(CV_light)<.5) return "Select Gradient or Soft Cel first.";
  if(index!=CV_lightmix && CV_Value(CV_lightmix)<=0) return "Increase Style blend first.";
  if(index!=CV_lightmix && index!=CV_worldlight && index!=CV_modellight &&
     !CV_Value(CV_worldlight) && !CV_Value(CV_modellight)) return "Enable World or Model lighting.";
  if((index==CV_bands||index==CV_softness) && CV_Value(CV_light)<1.5) return "Select Soft Cel to use cel bands.";
 }
 if(index>=CV_rimwidth && index<=CV_rimteam && CV_Value(CV_rim)<=0) return "Increase Player rim first.";
 if(index==CV_bloomthreshold||index==CV_bloomradius) { if(CV_Value(CV_bloom)<=0) return "Increase Bloom strength first."; }
 if(index==CV_aoradius && CV_Value(CV_ao)<=0) return "Increase World AO strength first.";
 if(index==CV_edgewidth||index==CV_edgestrength||index==CV_worldedge||index==EXT(2)) {
  if(!world) return "Enable World or Both outlines.";
  if(!RuleSets_AllowEdgeOutline()) return "World edges blocked by ruleset.";
  if(index!=CV_edgestrength && CV_Active() && CV_Value(CV_edgestrength)<=0) return "Increase World edge opacity first.";
 }
 if(index==CV_modelopacity||index==CV_teamedge||index==CV_enemyedge||index==EXT(1)||index==EXT(3)) {
  if(!model) return "Enable Models or Both outlines.";
  if(RuleSets_DisallowModelOutline(&playerModel)) return "Model outlines blocked by ruleset.";
  if(index!=EXT(1) && ExistingValue("gl_outline_scale_model")<=0) return "Increase Model edge width first.";
 }
 if(index==CV_edgepalette && !world && !model) return "Enable an Outline mode first.";
 if((index==CV_worldedge||index==CV_teamedge||index==CV_enemyedge) && !CV_Value(CV_edgepalette)) return "Enable Use edge palette first.";
 if(index==EXT(3) && CV_Active() && CV_Value(CV_edgepalette)) return "Disable Use edge palette first.";
 return NULL;
}
static void GreyText(int x,int y,const char *text) {
 char coloured[128]; snprintf(coloured,sizeof(coloured),"&c777%s&r",text);
 Draw_ColoredString(x+((menuwidth-320)>>1),y+m_yofs,coloured,0,false);
}

static void Bounds(int index, float *lo, float *hi, float *step) {
 if(index<CV_COUNT) { *lo=opts[index].lo; *hi=opts[index].hi; *step=opts[index].step; }
 else { const cv_existing_t *e=&existing[index-CV_COUNT]; *lo=e->lo; *hi=e->hi; *step=e->step; }
}
static qbool Toggle(int index) {
 float lo,hi,step; Bounds(index,&lo,&hi,&step); return lo==0 && hi==1 && step==1;
}
static qbool Slider(int index) {
 if(index==CV_RESTART_ROW) return false;
 if(Toggle(index)) return false;
 if(index<CV_COUNT) return index!=CV_light && index!=CV_tonemap;
 return existing[index-CV_COUNT].step>0 && existing[index-CV_COUNT].step<1;
}
static qbool SwitchedOn(cvar_t *v) {
 // Shared model-shadow admission uses integer, not an opacity value.
 return !strcmp(v->name,"r_shadows") ? v->integer!=0 : v->value!=0;
}
static void DrawControl(int index, int depth, int y) {
 if(index==CV_RESTART_ROW) {
  if(RestartPending()) M_Print(24,y,"Restart video (F5)"); else GreyText(24,y,"Restart video (F5)");
  return;
 }
 cvar_t *v=index<CV_COUNT?&vars[index]:Cvar_Find(existing[index-CV_COUNT].name);
 char label[24], value[32]; float lo,hi,step;
 qbool disabled=Unavailable(index)!=NULL;
 snprintf(label,sizeof(label),"%s%.*s",depth?"  ":"",depth?18:20,index<CV_COUNT?opts[index].label:existing[index-CV_COUNT].label);
 if(disabled) GreyText(24,y,label); else M_Print(24,y,label);
 if(!v) return;
 if(Slider(index)) {
  int i; float fraction;
  Bounds(index,&lo,&hi,&step);
  fraction=isfinite(v->value)?bound(0,(v->value-lo)/(hi-lo),1):0;
  if(disabled) {
   char track[]="[------]"; track[1+(int)(fraction*5)]='|'; GreyText(CV_SLIDER_X-8,y,track);
  } else {
   M_DrawCharacter(CV_SLIDER_X-8,y,128);
   for(i=0;i<6;++i) M_DrawCharacter(CV_SLIDER_X+i*8,y,129);
   M_DrawCharacter(CV_SLIDER_X+48,y,130);
   M_DrawCharacter(CV_SLIDER_X+CV_SLIDER_TRAVEL*fraction,y,131);
  }
  snprintf(value,sizeof(value),"%6.3g",v->value);
  if(disabled) GreyText(256,y,value); else M_PrintWhite(256,y,value);
 } else {
  const char *choice=NULL;
  if(Toggle(index)) choice=SwitchedOn(v)?"On":"Off";
  else if(index==CV_light) { const char *names[]={"Original","Gradient","Soft Cel"}; choice=names[(int)CV_Value(CV_light)]; }
  else if(index==CV_tonemap) { const char *names[]={"Original","Soft shoulder","Filmic"}; choice=names[(int)CV_Value(CV_tonemap)]; }
  else if(!strcmp(v->name,"gl_outline")) { const char *names[]={"Off","Models","World","Both"}; choice=names[(int)bound(0,v->value,3)]; }
  else if(!strcmp(v->name,"cl_muzzleflash")) { const char *names[]={"Off","On","Others only"}; choice=names[(int)bound(0,v->value,2)]; }
  else if(!strcmp(v->name,"gl_texturemode")) {
   if(!strcmp(v->string,"GL_NEAREST")) choice="Nearest";
   else if(!strcmp(v->string,"GL_LINEAR")) choice="Linear";
   else if(!strcmp(v->string,"GL_NEAREST_MIPMAP_NEAREST")) choice="Nearest mip";
   else if(!strcmp(v->string,"GL_LINEAR_MIPMAP_NEAREST")) choice="Bilinear";
   else if(!strcmp(v->string,"GL_NEAREST_MIPMAP_LINEAR")) choice="Nearest blend";
   else choice="Trilinear";
  }
  snprintf(value,sizeof(value),"%13.13s",choice?choice:v->string);
  if(disabled) GreyText(200,y,value); else M_PrintWhite(200,y,value);
 }
}

float CV_Value(cv_id id) {
 float value = vars[id].value;
 return isfinite(value) ? bound(opts[id].lo, value, opts[id].hi) : (float)atof(opts[id].def);
}
qbool CV_Active(void) { return CV_Value(CV_enable) != 0 && R_UseVulkan(); }
qbool CV_PostActive(void) {
 return R_UseVulkan() && (CV_Value(CV_bloom)>0 || CV_Value(CV_sharpen)>0 ||
  CV_Value(CV_tonemap)>0 || fabsf(CV_Value(CV_exposure)-1)>.0001);
}
qbool CV_RimAllowed(int effects, qbool player, qbool weapon, qbool blocked) {
 return player && !weapon && !blocked && !(effects & (EF_RED | EF_GREEN | EF_BLUE));
}
static void Palette(float *out, int index) {
 int i, c = 16 * bound(0,index,13) + 8;
 for (i=0;i<3;++i) out[i] = host_basepal[c*3+i] / 255.0f;
}
void CV_EdgeColor(float *out, int kind) {
 Palette(out,(int)CV_Value(kind==0?CV_worldedge:kind==1?CV_teamedge:CV_enemyedge));
}
void CV_Params(cv_params_t *out) {
 memset(out,0,sizeof(*out));
 out->world[0]=out->world[2]=out->world[3]=1;
 out->post[0]=1;
 out->modelview[0]=out->modelview[5]=out->modelview[10]=out->modelview[15]=1;
 // General image effects are independent of the competitive style switch.
 if(R_UseVulkan()) {
  out->post[0]=CV_Value(CV_exposure); out->post[1]=CV_Value(CV_tonemap);
  out->post[2]=CV_Value(CV_bloom); out->post[3]=CV_Value(CV_bloomthreshold);
  out->post2[0]=CV_Value(CV_bloomradius); out->post2[1]=CV_Value(CV_ao);
  out->post2[2]=CV_Value(CV_aoradius); out->post2[3]=CV_Value(CV_sharpen);
 }
 if (!CV_Active()) return;
 out->world[0]=CV_Value(CV_detail); out->world[1]=CV_Value(CV_pattern);
 out->world[2]=CV_Value(CV_contrast); out->world[3]=CV_Value(CV_saturation);
 Palette(out->floor,(int)CV_Value(CV_floor)); Palette(out->wall,(int)CV_Value(CV_wall));
 out->floor[3]=CV_Value(CV_tint); out->wall[3]=CV_Value(CV_distance);
 out->light[0]=CV_Value(CV_light); out->light[1]=CV_Value(CV_lightmix);
 out->light[2]=CV_Value(CV_worldlight); out->light[3]=CV_Value(CV_modellight);
 out->levels[0]=CV_Value(CV_shadow); out->levels[1]=CV_Value(CV_midtone);
 out->levels[2]=CV_Value(CV_highlight); out->levels[3]=CV_Value(CV_bands);
 out->shape[0]=CV_Value(CV_softness); out->shape[1]=CV_Value(CV_warmth);
 out->rim[1]=CV_Value(CV_rimwidth); out->rim[2]=CV_Value(CV_rimup);
}
void CV_PlayerParams(cv_params_t *out, entity_t *ent, int effects, int render_effects, const float *modelview) {
 int i; float tint;
 CV_Params(out);
 memcpy(out->modelview,modelview,sizeof(out->modelview));
 if(ent && RuleSets_DisallowModelOutline(ent->model)) out->light[3]=0;
 if (!CV_Active() || !ent || !CV_RimAllowed(effects, (render_effects & RF_PLAYERMODEL) != 0,
     (render_effects & RF_WEAPONMODEL) != 0, RuleSets_DisallowModelOutline(ent->model))) return;
 out->rim[0]=CV_Value(CV_rim);
 Palette(out->rimcolor,ent->scoreboard ? ent->scoreboard->topcolor : 0);
 tint=CV_Value(CV_rimteam);
 for(i=0;i<3;++i) out->rimcolor[i]=.65f*(1-tint)+out->rimcolor[i]*tint;
}
static cvar_t *Var(int index) { return index<CV_COUNT ? &vars[index] : Cvar_Find(existing[index-CV_COUNT].name); }
static void Snapshot(void) { int i; for(i=0;i<CV_TOTAL;++i) { cvar_t *v=Var(i); strlcpy(saved[i],v?v->string:"",sizeof(saved[i])); } }
static void Restore(void) { int i; for(i=0;i<CV_TOTAL;++i) { cvar_t *v=Var(i); if(v) Cvar_Set(v,saved[i]); } comparing=false; }
static void Reset(int which) {
 int i;
 for(i=0;i<CV_TOTAL;++i) {
  cvar_t *v=Var(i); int optionPage=i<CV_COUNT?opts[i].page:existing[i-CV_COUNT].page;
  if(v && (which==-1||optionPage==which||(which==-2&&optionPage>=1&&optionPage<=3))) Cvar_Set(v,v->defaultvalue);
 }
}
static void Preset(int n) {
 if(comparing) Restore();
 Reset(-2);
 if(n) {
  Cvar_SetValue(&vars[CV_enable],1); Cvar_SetValue(&vars[CV_detail],.35);
  Cvar_SetValue(&vars[CV_contrast],.8); Cvar_SetValue(&vars[CV_saturation],.85);
  Cvar_SetValue(&vars[CV_light],n==2?2:1); Cvar_SetValue(&vars[CV_lightmix],n==2?.6:.3);
  Cvar_SetValue(&vars[CV_rim],.06);
 }
 snprintf(notice,sizeof(notice),"Applied %s competitive style.",n==0?"Original":n==1?"Clean":"Soft Cel");
}
static void Compare(void) {
 if(comparing) { Restore(); strlcpy(notice,"Restored your visual settings.",sizeof(notice)); }
 else { Snapshot(); Reset(-2); comparing=true; strlcpy(notice,"Original comparison. F6 restores.",sizeof(notice)); }
}
static qbool ValidName(const char *s) {
 size_t n=strlen(s); if(!n||n>=sizeof(profile)) return false;
 while(*s) { if(!isalnum((unsigned char)*s)&&*s!='_'&&*s!='-') return false; ++s; } return true;
}
static void Profile(qbool load, const char *name) {
 char path[MAX_OSPATH], line[160], key[80], value[64], extra; FILE *f; int i, count=0;
 char pending[CV_TOTAL][64]; qbool seen[CV_TOTAL]={0}, valid=true;
 if(!ValidName(name)) { strlcpy(notice,"Use letters, digits, - or _ in names.",sizeof(notice)); return; }
 snprintf(path,sizeof(path),"%s/ezquake/competitive/%s.cfg",com_basedir,name);
 if(!load) {
  if(comparing) Restore();
  FS_CreatePath(path); f=fopen(path,"w");
  if(!f) { strlcpy(notice,"Could not create visual profile.",sizeof(notice)); return; }
  fprintf(f,"// Competitive Visuals profile v1\n");
  for(i=0;i<CV_TOTAL;++i) { cvar_t *v=Var(i); if(v) fprintf(f,"%s %s\n",v->name,v->string); }
  valid=!ferror(f); if(fclose(f)) valid=false;
 } else {
  f=fopen(path,"r"); if(!f) { strlcpy(notice,"Visual profile not found.",sizeof(notice)); return; }
  while(fgets(line,sizeof(line),f)) {
   char *end; double number;
   if(++count>256 || (!strchr(line,'\n')&&!feof(f))) { valid=false; break; }
   if(line[0]=='/'&&line[1]=='/') continue;
   if(sscanf(line,"%79s %63s %c",key,value,&extra)!=2) { valid=false; break; }
   for(i=0;i<CV_TOTAL;++i) { cvar_t *v=Var(i); if(v&&!strcmp(v->name,key)) break; }
   if(i==CV_TOTAL) { valid=false; break; }
   if(!strcmp(key,"gl_texturemode")) {
    if(strcmp(value,"GL_NEAREST")&&strcmp(value,"GL_LINEAR")&&strcmp(value,"GL_NEAREST_MIPMAP_NEAREST")&&strcmp(value,"GL_LINEAR_MIPMAP_NEAREST")&&strcmp(value,"GL_NEAREST_MIPMAP_LINEAR")&&strcmp(value,"GL_LINEAR_MIPMAP_LINEAR")) { valid=false; break; }
   } else {
    number=strtod(value,&end);
    if(*end||!isfinite(number)) { valid=false; break; }
    // Validate at the same float precision used by cvars and renderer bounds.
    // Otherwise decimal endpoints such as 0.2 compare below their float minimum.
    if(i<CV_COUNT && ((float)number<opts[i].lo||(float)number>opts[i].hi)) { valid=false; break; }
    if(i>=CV_COUNT && ((float)number<existing[i-CV_COUNT].lo||(float)number>existing[i-CV_COUNT].hi)) { valid=false; break; }
   }
   strlcpy(pending[i],value,sizeof(pending[i])); seen[i]=true;
  }
  if(ferror(f)) valid=false; fclose(f);
  { qbool any=false; for(i=0;i<CV_TOTAL;++i) any |= seen[i]; valid &= any; }
  if(valid) {
   if(comparing) Restore();
   for(i=0;i<CV_TOTAL;++i) if(seen[i]) Cvar_Set(Var(i),pending[i]);
  }
 }
 snprintf(notice,sizeof(notice),"%s",!valid?"Invalid profile; no settings loaded.":load?"Loaded visual-only profile.":"Saved visual-only profile.");
 Con_Printf("CV_PROFILE %s %s: %s\n",load?"load":"save",name,notice);
}
static void BuildRows(void) {
 int i,lastGroup=-1; rows=0;
 if(!page) { rows=8; return; }
 for(i=0;i<sizeof(layout)/sizeof(layout[0]);++i) if(layout[i].page==page && (layout[i].index==CV_RESTART_ROW||Var(layout[i].index))) {
  if(lastGroup!=layout[i].group) { lastGroup=layout[i].group; indices[rows++]=-lastGroup-1; }
  indices[rows]=layout[i].index; depths[rows++]=layout[i].depth;
 }
 cursor=bound(0,cursor,max(0,rows-1));
 while(cursor<rows-1 && indices[cursor]<0) ++cursor;
}
static int ViewEnd(int first) {
 int end=min(rows,first+CV_VISIBLE_ROWS);
 // Keep section titles with their first control across viewport boundaries.
 if(page && end<rows && indices[end-1]<0) --end;
 return end;
}
static int ViewFirst(void) {
 int first=0;
 while(cursor>=ViewEnd(first)) first=ViewEnd(first);
 return first;
}
void CV_Open(void) { general=false; page=0; cursor=0; dragging=-1; M_EnterMenu(m_competitive); BuildRows(); }
void CV_OpenEffects(void) { general=true; page=4; cursor=0; dragging=-1; M_EnterMenu(m_competitive); BuildRows(); }
static const char *Help(int index) {
 if(index==CV_RESTART_ROW) return "Apply pending graphics changes by restarting video. The game stays loaded.";
 return index<CV_COUNT?opts[index].help:existing[index-CV_COUNT].help;
}
static const char *profileHelp[]={
 "Restore the original competitive look. General image effects are preserved.",
 "Reduce fine world detail. Keep broad patterns and original light structure.",
 "Apply softly blended cel lighting and a small player rim.",
 "Name for saving/loading all visual settings. Letters, digits, - and _ only.",
 "Save competitive and general visual settings together. Keys are excluded.",
 "Load the named visual profile. Invalid files leave your settings unchanged.",
 "Compare with the original competitive look. Press F6 again to restore.",
 "Reset competitive controls. General image effects are preserved."
};
static void DrawHelp(const char *text) {
 int line;
 for(line=0;line<3 && *text;++line) {
  char part[37]; size_t n=min(strlen(text),36), cut=n;
  if(text[n] && text[n]!=' ') { while(cut && text[cut]!=' ') --cut; if(cut) n=cut; }
  memcpy(part,text,n); part[n]=0; M_PrintWhite(16,146+line*10,part);
  text+=n; while(*text==' ') ++text;
 }
}
void CV_Draw(void) {
 int i,first,end; char line[80];
 M_PrintWhite(64,8,general?"VISUAL EFFECTS":"COMPETITIVE VISUALS");
 snprintf(line,sizeof(line),"< %s >  %d/%d",pages[page],general?page-3:page+1,general?2:4); M_PrintWhite(80,24,line);
 BuildRows(); first=ViewFirst(); end=ViewEnd(first);
 if(notice[0]) snprintf(line,sizeof(line),"%d-%d/%d  %.24s",first+1,end,rows,notice);
 else snprintf(line,sizeof(line),"%s  rows %d-%d/%d",R_UseVulkan()?"Vulkan":"OpenGL",first+1,end,rows);
 M_PrintWhite(16,36,line);
 for(i=first;i<end;++i) {
  int y=52+(i-first)*10;
  if(!page) {
   const char *labels[]={"Original preset","Clean preset","Soft Cel preset",NULL,"Save visual profile","Load visual profile","Compare original (F6)","Reset visual style"};
   if(i==3) snprintf(line,sizeof(line),"Name: %.28s%s",profile,editing?"_":""); else strlcpy(line,labels[i],sizeof(line));
  } else {
   line[0]=0;
  }
  if(i==first) M_Print_GetPoint(24,y,&window.x,&window.y,line,false); else if(!page) M_Print(24,y,line);
  if(page) {
   if(indices[i]<0) M_PrintWhite(24,y,(char *)groups[-indices[i]-1]);
   else DrawControl(indices[i],depths[i],y);
  }
 }
 window.w=280; window.h=(end-first)*10;
 M_DrawCharacter(16,52+(cursor-first)*10,FLASHINGARROW());
 if(RestartPending()) M_Print(16,136,"Video restart required - F5 applies");
 DrawHelp(page?Help(indices[cursor]):profileHelp[cursor]);
 if(page) {
  const char *why=Unavailable(indices[cursor]);
  if(why) { snprintf(line,sizeof(line),"%.36s",why); GreyText(16,180,line); }
  else if(indices[cursor]==EXT(7) && R_UseVulkan()) {
   snprintf(line,sizeof(line),"Active MSAA: %dx%s",actualMSAA,RestartPending()?"; change pending":""); M_PrintWhite(16,180,line);
  }
 }
 M_PrintWhite(16,190,general?"PgUp/Dn: page  F7: reset page":"PgUp/Dn: page  F6: compare  F7: reset");
}
void CV_Key(int key) {
 int index,direction=key==K_LEFTARROW?-1:1; cvar_t *v;
 dragging=-1;
 if(editing) {
  size_t len=strlen(profile);
  if(key==K_ENTER||key==K_ESCAPE) editing=false;
  else if(key==K_BACKSPACE&&len) profile[len-1]=0;
  else if(key>0&&key<128&&len+1<sizeof(profile)&&(isalnum(key)||key=='_'||key=='-')) { profile[len]=(char)key; profile[len+1]=0; }
  return;
 }
 BuildRows();
 if(key==K_F5) { if(RestartPending()) Cbuf_InsertText("vid_restart\n"); return; }
 if(key==K_ESCAPE||key==K_MOUSE2) { M_LeaveMenu(m_options); return; }
 if(key==K_F6) { if(!general) Compare(); return; }
 if(key==K_F7) { if(comparing) Restore(); Reset(page?page:-2); strlcpy(notice,"Restored default visual settings.",sizeof(notice)); return; }
 if(key==K_PGUP||key==K_PGDN||key==K_TAB) {
  int count=general?2:4, base=general?4:0;
  notice[0]=0; page=base+(page-base+(key==K_PGUP?count-1:1))%count; cursor=0; BuildRows(); return;
 }
 if(key==K_UPARROW||key==K_MWHEELUP||key==K_DOWNARROW||key==K_MWHEELDOWN) {
  int step=(key==K_UPARROW||key==K_MWHEELUP)?-1:1;
  do { cursor=(cursor+rows+step)%rows; } while(page && indices[cursor]<0); return;
 }
 if(key==K_HOME) { cursor=0; BuildRows(); return; } if(key==K_END) { cursor=rows-1; return; }
 if(key!=K_ENTER&&key!=K_MOUSE1&&key!=K_LEFTARROW&&key!=K_RIGHTARROW) return;
 if(!page) {
  if(cursor<3) Preset(cursor);
  else if(cursor==3) editing=true;
  else if(cursor==4||cursor==5) Profile(cursor==5,profile);
  else if(cursor==6) Compare();
  else { if(comparing) Restore(); Reset(-2); }
  return;
 }
 index=indices[cursor];
 if(index==CV_RESTART_ROW) { CV_Key(K_F5); return; }
 if(Unavailable(index) && !comparing) return;
 if(comparing) Restore();
 if(Unavailable(index)) return;
 v=Var(index); if(!v) return;
 if(index<CV_COUNT) {
  float value=CV_Value(index);
  if((key==K_ENTER||key==K_MOUSE1)&&opts[index].lo==0&&opts[index].hi==1&&opts[index].step==1) value=1-value;
  else value=bound(opts[index].lo,value+direction*opts[index].step,opts[index].hi);
  Cvar_SetValue(v,value);
 }
 else {
  const cv_existing_t *e=&existing[index-CV_COUNT];
  if(!strcmp(e->name,"gl_texturemode")) {
   const char *filters[]={"GL_NEAREST","GL_LINEAR","GL_NEAREST_MIPMAP_NEAREST","GL_LINEAR_MIPMAP_NEAREST","GL_NEAREST_MIPMAP_LINEAR","GL_LINEAR_MIPMAP_LINEAR"}; int i;
   for(i=0;i<6;++i) if(!strcmp(v->string,filters[i])) break;
   Cvar_Set(v,(char *)filters[(i+6+direction)%6]);
  } else if(!strcmp(e->name,"vid_framebuffer_multisample")) {
   int samples[]={0,2,4,8}, i=0;
   while(i<3 && samples[i]<v->value) ++i;
   Cvar_SetValue(v,samples[bound(0,i+direction,3)]);
  } else if(Toggle(index) && (key==K_ENTER||key==K_MOUSE1)) Cvar_SetValue(v,!SwitchedOn(v));
  else Cvar_SetValue(v,bound(e->lo,v->value+direction*e->step,e->hi));
 }
}
qbool CV_Mouse(const mouse_state_t *ms) {
 int first,selected; qbool inside;
 double x=ms->x*(scr_scaleMenu.value?(double)menuwidth/vid.width:1);
 double y=ms->y*(scr_scaleMenu.value?(double)menuheight/vid.height:1);
 double track=window.x+CV_SLIDER_X-24;
 if(ms->button_up==2) { CV_Key(K_MOUSE2); return true; }
 if(editing) return true;
 BuildRows(); first=ViewFirst(); selected=cursor-first;
 inside=M_Mouse_Select(&window,ms,ViewEnd(first)-first,&selected);
 if(inside && page && indices[first+selected]<0) inside=false;
 if(dragging<0 && inside) cursor=first+selected;
 if(dragging<0 && inside && page && !Unavailable(indices[cursor]) && Slider(indices[cursor]) &&
    x>=track-8 && x<=track+CV_SLIDER_TRAVEL+16 && ms->button_down==1) dragging=indices[cursor];
 if(dragging>=0) {
  if(Unavailable(dragging)) { dragging=-1; return true; }
  if(ms->buttons[1] || ms->button_down==1 || ms->button_up==1) {
   float lo,hi,step,value;
   Bounds(dragging,&lo,&hi,&step);
   value=lo+bound(0,(x-track)/CV_SLIDER_TRAVEL,1)*(hi-lo);
   value=lo+floorf((value-lo)/step+.5f)*step;
   if(comparing) Restore();
   Cvar_SetValue(Var(dragging),bound(lo,value,hi));
  }
  if(ms->button_up==1 || (!ms->buttons[1] && ms->button_down!=1)) dragging=-1;
  return true;
 }
 if(ms->button_up==1) {
  if(y>=window.y-28 && y<window.y-20 && x>=window.x+56 && x<=window.x+216)
   CV_Key(x<window.x+136?K_PGUP:K_PGDN);
  else if(inside) CV_Key(K_MOUSE1);
 }
 return true;
}
static void Command(void) {
 const char *action=Cmd_Argv(1);
 if(!strcmp(action,"preset")) Preset(!strcmp(Cmd_Argv(2),"clean")?1:!strcmp(Cmd_Argv(2),"cel")?2:0);
 else if(!strcmp(action,"save")||!strcmp(action,"load")) Profile(!strcmp(action,"load"),Cmd_Argv(2));
 else if(!strcmp(action,"compare")) Compare();
 else Con_Printf("cv preset original|clean|cel; cv save|load <name>; cv compare\n");
}
// Opt-in local screenshot fixture. No effect without both -dev and -visual-tests.
static qbool photoActive;
static double photoTime;
static vec3_t photoOrigin, photoAngles;
static qbool PhotoAllowed(void) {
#ifndef CLIENTONLY
 const netadr_t *address=&cls.netchan.remote_address;
 return IsDeveloperMode() && COM_FindParm("-visual-tests") && sv.state==ss_active &&
  cls.state==ca_active && r_refdef2.allow_cheats &&
  (address->type==NA_LOOPBACK || (address->type==NA_IP && address->ip[0]==127));
#else
 return false;
#endif
}
void CV_TestView(void) {
 if(!photoActive || !PhotoAllowed()) return;
 cl.time=photoTime; r_refdef2.time=photoTime;
 VectorCopy(photoOrigin,r_refdef.vieworg); VectorCopy(photoAngles,r_refdef.viewangles);
}
static void PhotoCommand(void) {
#ifndef CLIENTONLY
 const char *action=Cmd_Argv(2); int i;
 if(!PhotoAllowed()) { Con_Printf("CV_PHOTO rejected: requires opt-in local cheat-enabled test server.\n"); return; }
 if(!strcmp(action,"freeze")) {
  VectorCopy(r_refdef.vieworg,photoOrigin); VectorCopy(r_refdef.viewangles,photoAngles);
  photoTime=cl.time; if(!sv.paused) SV_TogglePause(NULL,1); cl.paused|=PAUSED_SERVER; photoActive=true;
 } else if(!strcmp(action,"camera") && Cmd_Argc()==9) {
  for(i=0;i<3;++i) { photoOrigin[i]=(float)atof(Cmd_Argv(3+i)); photoAngles[i]=(float)atof(Cmd_Argv(6+i)); }
 } else if(!strcmp(action,"actors") && photoActive) {
  vec3_t forward,right;
  model_t *player=cl.model_precache[cl_modelindices[mi_player]], *ordinary=NULL;
  if(!player || cl.num_statics+3>=MAX_STATIC_ENTITIES) return;
  for(i=1;i<MAX_MODELS;++i) if(cl.model_precache[i] && strstr(cl.model_precache[i]->name,"backpack")) { ordinary=cl.model_precache[i]; break; }
  AngleVectors(photoAngles,forward,right,NULL);
  for(i=0;i<3;++i) {
   entity_t *ent;
   if(i==2 && !ordinary) continue;
   ent=&cl_static_entities[cl.num_statics++]; memset(ent,0,sizeof(*ent));
   ent->entity_id=cl.num_statics; ent->model=i<2?player:ordinary; ent->colormap=vid.colormap;
   VectorMA(photoOrigin,80,forward,ent->origin); VectorMA(ent->origin,(i-1)*30,right,ent->origin);
   ent->origin[2]=photoOrigin[2]-22; ent->angles[1]=photoAngles[1]+135;
   if(i<2) {
    int slot=MAX_CLIENTS-1-i;
    cl.players[slot]=cl.players[cl.playernum]; cl.players[slot].teammate=i==0;
    cl.players[slot].topcolor=i?13:4; cl.players[slot].bottomcolor=3;
    ent->scoreboard=&cl.players[slot]; ent->renderfx=RF_PLAYERMODEL;
   }
   R_AddEfrags(ent);
  }
 }
 Con_Printf("CV_PHOTO active=%d origin=%g,%g,%g angles=%g,%g,%g statics=%d time=%g\n",photoActive,
  photoOrigin[0],photoOrigin[1],photoOrigin[2],photoAngles[0],photoAngles[1],photoAngles[2],cl.num_statics,cl.time);
#endif
}
static void Developer(void) {
 if(!developer.value) return;
 if(!strcmp(Cmd_Argv(1),"runtime")) {
  int i,players=0;
  for(i=0;i<MAX_CLIENTS;++i) if(cl.players[i].name[0] && !cl.players[i].spectator) ++players;
  Con_Printf("CV_RUNTIME map=%s state=%d demo=%d time=%.3f seeking=%d players=%d vulkan=%d msaa=%d\n",
   cl.worldmodel?cl.worldmodel->name:"none",cls.state,cls.demoplayback,cls.demotime,cls.demoseeking,players,R_UseVulkan(),actualMSAA);
  return;
 }
 if(!strcmp(Cmd_Argv(1),"photo")) { PhotoCommand(); return; }
 if(!strcmp(Cmd_Argv(1),"checkpoint")) { cbuf_main.runAwayLoop=cbuf_main.waitCount=0; return; }
 if(!strcmp(Cmd_Argv(1),"key")) CV_Key(Key_StringToKeynum(Cmd_Argv(2)));
 if(!strcmp(Cmd_Argv(1),"page")) { page=bound(0,atoi(Cmd_Argv(2)),5);general=page>=4;cursor=0;BuildRows(); }
 if(!strcmp(Cmd_Argv(1),"select")) {
  int i;
  for(i=0;i<CV_TOTAL;++i) if(Var(i)&&!strcmp(Var(i)->name,Cmd_Argv(2))) {
   int row,target=i<CV_COUNT?opts[i].page:existing[i-CV_COUNT].page;
   if(target<0) { Con_Printf("CV_MOVED %s: %s\n",Var(i)->name,Help(i)); return; }
   page=target; general=page>=4; BuildRows();
   for(row=0;row<rows;++row) if(indices[row]==i) cursor=row;
   break;
  }
 }
 if(!strcmp(Cmd_Argv(1),"mouse")) {
  mouse_state_t ms={0};
  ms.x=(window.x+CV_SLIDER_X-24+atof(Cmd_Argv(3))*CV_SLIDER_TRAVEL)*(scr_scaleMenu.value?(double)vid.width/menuwidth:1);
  ms.y=(window.y+(cursor-ViewFirst())*10+4)*(scr_scaleMenu.value?(double)vid.height/menuheight:1);
  ms.button_down=!strcmp(Cmd_Argv(2),"down")?1:0;
  ms.button_up=!strcmp(Cmd_Argv(2),"up")?1:0;
  ms.buttons[1]=!ms.button_up; CV_Mouse(&ms);
 }
 Con_Printf("CV_STATE page=%d rows=%d active=%d compare=%d rim=%g bloom=%g\n",page,rows,CV_Active(),comparing,CV_Value(CV_rim),CV_Value(CV_bloom));
 if(page) Con_Printf("CV_HELP %s enabled=%d: %s | %s\n",indices[cursor]==CV_RESTART_ROW?"restart_video":Var(indices[cursor])->name,!Unavailable(indices[cursor]),Help(indices[cursor]),Unavailable(indices[cursor])?Unavailable(indices[cursor]):"Ready");
 else Con_Printf("CV_HELP profile%d: %s\n",cursor,profileHelp[cursor]);
 if(!strcmp(Cmd_Argv(1),"params")) {
  cv_params_t params; CV_Params(&params);
  Con_Printf("CV_PARAMS style=%d post=%d bloom=%g exposure=%g light=%g ao=%g\n",CV_Active(),CV_PostActive(),params.post[2],params.post[0],params.light[0],params.post2[1]);
 }
 Con_Printf("CV_VIDEO pending=%d requested=%g applied=%d actual=%d\n",RestartPending(),ExistingValue("vid_framebuffer_multisample"),appliedMSAA,actualMSAA);
}
void CV_Init(void) {
 int i; for(i=0;i<CV_COUNT;++i) Cvar_Register(&vars[i]);
 Cmd_AddCommand("menu_competitive",CV_Open); Cmd_AddCommand("cv",Command);
 Cmd_AddCommand("menu_visual_effects",CV_OpenEffects);
 if(IsDeveloperMode()) Cmd_AddCommand("dev_competitive",Developer);
}
