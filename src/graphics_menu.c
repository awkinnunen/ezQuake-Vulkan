/* MENU-UNIFY-001 / CFG-PRESETS-001, OpenAI Codex, 2026-09-15.
 * GPL-2.0-or-later. Declarative graphics presets; never execute CFG commands. */
#include "quakedef.h"
#include "gl_model.h"
#include "tr_types.h"
#include "r_local.h"
#include "graphics_menu.h"
#include "settings_page.h"
#include "menu.h"
#include <ctype.h>
#include <math.h>

#define GFX_MAX 256
#define GFX_FILES 128
#define GFX_VALUE 256
static const char *categories[]={"All graphics","Image quality","World and textures","Lighting and shadows","Players and weapons","Particles and effects","Competitive clarity","Camera and view"};
typedef struct {setting row;int category;const char *section;} graphics_option_t;
typedef struct {char name[64],label[96],path[MAX_OSPATH];qbool readonly;} graphics_preset_t;
static graphics_option_t options[GFX_MAX];
static int optionCount,category=1,scope,presetCount,previewIndex=-1,presetStart;
static setting *original;static int originalCount;
static const char *addingSection="";
static setting rows[GFX_MAX*2+16],presetRows[GFX_FILES+16];
static settings_page settings,presets;
static graphics_preset_t files[GFX_FILES];
static char baseline[GFX_MAX][GFX_VALUE],applied[GFX_MAX][GFX_VALUE],pending[GFX_MAX][GFX_VALUE];
static char activeName[64]="Current settings",filename[64]="my-graphics",notice[192];
static qbool initialized,browsing,preview,typing,rebuild;
static const char *videoNames[]={"gl_max_size","gl_miptexLevel","vid_vulkan_antilag"};
static char videoValues[3][64];static qbool videoCaptured;
void Graphics_VideoApplied(void){int i;videoCaptured=true;for(i=0;i<3;++i){cvar_t *v=Cvar_Find(videoNames[i]);if(v)strlcpy(videoValues[i],v->string,sizeof(videoValues[i]));else videoCaptured=false;}}
static qbool VideoPending(void){int i;if(CV_RestartPending())return true;if(R_UseVulkan()&&videoCaptured)for(i=0;i<3;++i){cvar_t *v=Cvar_Find(videoNames[i]);if(v&&strcmp(v->string,videoValues[i]))return true;}return false;}
static int mouseTop,mouseLeft;
static void BuildRows(void);static void OpenPresets(void);static void Developer(void);static void Command(void);
static void SaveCurrent(void);static void SaveNew(void);static qbool WritePreset(const char *name,qbool overwrite);
static void IdentifyBuiltIn(void);
static const char *Value(cvar_t *v){return v->latchedString?v->latchedString:v->string;}
static qbool InScope(int i){return !scope||options[i].category==scope;}
static void Capture(char state[GFX_MAX][GFX_VALUE]){int i;for(i=0;i<optionCount;++i)strlcpy(state[i],Value(options[i].row.cvar),GFX_VALUE);}
static void Restore(char state[GFX_MAX][GFX_VALUE]){int i;for(i=0;i<optionCount;++i)if(strcmp(Value(options[i].row.cvar),state[i]))Cvar_Set(options[i].row.cvar,state[i]);}
static int Changed(char state[GFX_MAX][GFX_VALUE]){int i,n=0;for(i=0;i<optionCount;++i)n+=strcmp(Value(options[i].row.cvar),state[i])!=0;return n;}
static int Find(const char *name){int i;for(i=0;i<optionCount;++i)if(!strcasecmp(options[i].row.cvar->name,name))return i;return -1;}
static qbool SafeName(const char *s){int i;if(!*s||strlen(s)>48)return false;for(i=0;s[i];++i)if(!isalnum((unsigned char)s[i])&&s[i]!='-'&&s[i]!='_')return false;return true;}
static void Add(setting s,int group){if(!s.cvar||Find(s.cvar->name)>=0||optionCount>=GFX_MAX)return;s.advanced=false;if(s.type==stt_num&&(!*s.cvar->string||strchr(s.cvar->string,' ')))s.type=stt_string;if(s.type==stt_bool&&(atof(s.cvar->defaultvalue)>1||s.cvar->value>1)){s.type=stt_num;s.min=0;s.max=max(1,s.cvar->value);s.step=1;}if(s.type==stt_num){float def=(float)atof(s.cvar->defaultvalue);s.min=min(s.min,min(def,s.cvar->value));s.max=max(s.max,max(def,s.cvar->value));}options[optionCount].row=s;options[optionCount].section=addingSection;options[optionCount++].category=group;}
static int CVCategory(const char *group){
 if(!strcmp(group,"Bloom")||!strcmp(group,"Scene image")||!strcmp(group,"Antialiasing / filtering"))return 1;
 if(!strcmp(group,"Ambient shading")||!strcmp(group,"Light and shadow maps"))return 3;
 if(!strcmp(group,"Surface decoration"))return 2;return 6;
}
static int OriginalCategory(const char *group){
 if(!strcmp(group,"Field Of View"))return 7;if(!strcmp(group,"Player & Weapon Model"))return 4;
 if(!strcmp(group,"Lighting"))return 3;if(!strcmp(group,"Projectiles")||!strcmp(group,"Explosions"))return 5;return 2;
}
static const char *Policy(const setting *s){
 if(s->type!=stt_intnum&&s->cvar&&R_UseVulkan()&&(!strcmp(s->cvar->name,"vid_framebuffer")||!strcmp(s->cvar->name,"vid_framebuffer_hdr")||!strcmp(s->cvar->name,"vid_framebuffer_hdr_tonemap")))return "OpenGL only. Vulkan HDR is in Graphics / Image quality.";
 if(s->type!=stt_intnum && s->cvar && Find(s->cvar->name)>=0){
  if((s->cvar->flags&CVAR_ROM))return "This setting is read-only.";
  return CV_NativeUnavailable(s);
 }return NULL;
}
static void Ensure(void){
 setting cv[100];int i,n,group=2;
 if(initialized)return;initialized=true;
 n=CV_NativeSettings(cv,100);
 for(i=0;i<n;++i){if(cv[i].type==stt_separator){addingSection=cv[i].label;group=CVCategory(cv[i].label);}else Add(cv[i],group);}
 group=2;
 for(i=0;i<originalCount;++i){setting s=original[i];if(s.type==stt_separator){addingSection=s.label;group=OriginalCategory(s.label);}else if(s.cvar&&strcmp(s.cvar->name,"menu_advanced"))Add(s,group);}
 /* Extra legacy particle settings are added below from a fixed graphics allowlist. */
 {
  static const char *names[]={"cl_camera_death",
"cl_camera_tpp",
"cl_camera_tpp_distance",
"cl_camera_tpp_height",
"cl_hidenails",
"cl_hiderockets",
"gl_buildingsparks",
"gl_coronas",
"gl_coronas_tele",
"gl_cutf_tesla_effect",
"gl_detpacklights",
"gl_extratrails",
"gl_inferno_speed",
"gl_inferno_trail",
"gl_lighting_color",
"gl_lighting_vertex",
"gl_lightning",
"gl_lightning_color",
"gl_lightning_size",
"gl_lightning_sparks",
"gl_lightning_sparks_size",
"gl_nailtrail",
"gl_nailtrail_plasma",
"gl_nailtrail_turb",
"gl_particle_blobs",
"gl_particle_blood",
"gl_particle_blood_color",
"gl_particle_blood_type",
"gl_particle_deatheffect",
"gl_particle_explosions",
"gl_particle_fire",
"gl_particle_firecolor",
"gl_particle_gibtrails",
"gl_particle_gunshots",
"gl_particle_gunshots_type",
"gl_particle_muzzleflash",
"gl_particle_shockwaves",
"gl_particle_shockwaves_flat",
"gl_particle_sparks",
"gl_particle_spikes",
"gl_particle_spikes_type",
"gl_particle_telesplash",
"gl_particle_trail_detail",
"gl_particle_trail_length",
"gl_particle_trail_time",
"gl_particle_trail_type",
"gl_particle_trail_width",
"gl_surface_lava",
"gl_surface_slime",
"gl_turb_effects",
"gl_turb_fire",
"gl_turb_trails",
"gl_weather_rain",
"gl_weather_rain_fast",
"r_damagestats"};
  static char labels[sizeof(names)/sizeof(names[0])][80],helps[sizeof(names)/sizeof(names[0])][160];
  addingSection="Particle detail";
  for(i=0;i<sizeof(names)/sizeof(names[0]);++i){cvar_t *v=Cvar_Find(names[i]);setting e={0};int k;
   if(!v)continue;e.cvar=v;e.type=stt_num;e.min=-128;e.max=128;e.step=.05f;
   if(v->flags&CVAR_COLOR || strstr(v->name,"color") || !*v->string){e.type=stt_string;}
   else if(!strcmp(v->defaultvalue,"0")||!strcmp(v->defaultvalue,"1")){e.min=0;e.max=16;e.step=.1f;}
   if(!strcmp(v->name,"gl_particle_trail_detail")){e.min=1;e.max=100;e.step=1;}
   {const char *name=v->name;if(!strncmp(name,"gl_",3)||!strncmp(name,"cl_",3))name+=3;else if(!strncmp(name,"r_",2))name+=2;if(!strncmp(name,"particle_",9))name+=9;snprintf(labels[i],sizeof(labels[i]),"%s",name);}
   for(k=0;labels[i][k];++k)if(labels[i][k]=='_')labels[i][k]=' ';labels[i][0]=(char)toupper((unsigned char)labels[i][0]);
   snprintf(helps[i],sizeof(helps[i]),"Legacy particle/effect control: %s. Requires the installed particle resources.",v->name);e.label=labels[i];e.description=helps[i];Add(e,!strncmp(v->name,"cl_camera",9)?7:5);
  }
  {static const char *extra[]={"gl_gamma","gl_contrast","gl_modulate","gl_anisotropy","vid_vsync","vid_vulkan_antilag","vid_framebuffer_scale"};
   static const char *label[]={"Display gamma","Display contrast","Lightmap intensity","Anisotropic filtering","Vertical sync","Reduce input lag","Render scale"};
   static const float lo[]={.3f,1,.5f,0,0,0,.25f},hi[]={3,5,3,16,1,1,2},step[]={.1f,.1f,.1f,1,1,1,.25f};
   addingSection="Display and filtering";
   for(i=0;i<7;++i){setting e={0};e.cvar=Cvar_Find(extra[i]);e.type=i==4||i==5?stt_bool:stt_num;e.label=label[i];e.description=i==5?"Vulkan latency reduction; requires driver support.":"Scene/display control shared with the original renderer. Video changes may require a restart.";e.min=lo[i];e.max=hi[i];e.step=step[i];Add(e,i==2?3:1);}
  }
 }

 rows[0].type=presetRows[0].type=stt_action;rows[0].label=presetRows[0].label="Graphics";
 Capture(applied);Settings_Init(&settings,rows,1,"unified_graphics");Settings_Init(&presets,presetRows,1,"graphics_presets");
 BuildRows();IdentifyBuiltIn();
}
static void SetAction(setting *s,const char *label,action_fnc action,const char *help){memset(s,0,sizeof(*s));s->type=stt_action;s->label=label;s->actionfnc=action;s->description=help;}
static const char *CategoryName(void){static const char *shortNames[]={"All","Image quality","World/textures","Light/shadows","Players/weapons","Particles/effects","Competitive","Camera/view"};return shortNames[category];}
static void CategoryNext(qbool back){category=1+(category-1+(back?6:1))%7;rebuild=true;}
static void Restart(void){if(browsing){strlcpy(notice,"Apply or cancel the preview before restarting video.",sizeof(notice));return;}Cbuf_AddText("vid_restart\n");}
static void BuildRows(void){
 int i,n=0;const char *section="";memset(rows,0,sizeof(rows));
 SetAction(&rows[n++],"Browse graphics presets (F2)",OpenPresets,"Highlight a named CFG to preview; Enter or click loads it, Escape cancels. Save all graphics or the selected category.");
 rows[n].type=stt_custom;rows[n].label="Category";rows[n].readfnc=CategoryName;rows[n].togglefnc=CategoryNext;rows[n++].description="Use Left/Right to choose a graphics category. All categories use the same controls and preset browser.";
 SetAction(&rows[n++],"Restart video (F5)",Restart,"Apply pending HDR/MSAA and video settings. Preset browsing never restarts video automatically.");
 SetAction(&rows[n++],"Save current preset",SaveCurrent,"Save edits to the active user preset, preserving a backup. Built-in presets require a new name.");
 SetAction(&rows[n++],"Save as new preset",SaveNew,"Save current graphics under a new name. Controls, audio, HUD and match rules are excluded.");
 rows[n].type=stt_separator;rows[n++].label=categories[category];
 for(i=0;i<optionCount;++i)if(options[i].category==category){if(strcmp(section,options[i].section)){section=options[i].section;rows[n].type=stt_separator;rows[n++].label=section;}rows[n++]=options[i].row;}
 settings.count=n;settings.settings=rows;settings.marked=1;settings.viewpoint=0;Settings_OnShow(&settings);rebuild=false;
}
void Graphics_Init(setting *source,int count){original=source;originalCount=count;Settings_AddPolicy(Policy);Cmd_AddCommand("menu_graphics",Graphics_Open);Cmd_AddCommand("gfx",Command);if(IsDeveloperMode())Cmd_AddCommand("dev_graphics",Developer);}
void Graphics_Shutdown(void){Graphics_CancelPreview();if(initialized){Settings_Shutdown(&settings);Settings_Shutdown(&presets);}initialized=false;optionCount=0;}
void Graphics_OnShow(void){Ensure();Settings_OnShow(browsing?&presets:&settings);}
void Graphics_CancelPreview(void){if(browsing){Restore(baseline);browsing=preview=typing=false;previewIndex=-1;strlcpy(notice,"Preview cancelled; previous graphics restored.",sizeof(notice));}}
static qbool ValidValue(int i,const char *value){
 setting *s=&options[i].row;char *end;double n;int k;
 if(strlen(value)>=GFX_VALUE||strpbrk(value,";\r\n"))return false;
 if(s->type==stt_enum){for(k=0;k<=(int)s->max;++k)if(!strcasecmp(value,s->named_ints[k*2+1]))return true;return false;}
 if(s->type==stt_string){/* Graphics-only color strings, never a console script. */for(k=0;value[k];++k)if(!isalnum((unsigned char)value[k])&&!strchr(" ._-+",value[k]))return false;return true;}
 n=strtod(value,&end);if(end==value||*end||!isfinite(n))return false;
 if(s->type==stt_bool)return n==0||n==1;
 return (float)n>=s->min&&(float)n<=s->max;
}
static qbool ReadPreset(int file){
 FILE *f;char line[1024];int count=0,total=0; qbool seen[GFX_MAX]={0},valid=true;
 memcpy(pending,baseline,sizeof(pending));
 f=fopen(files[file].path,"rb");if(!f)return false;
 while(fgets(line,sizeof(line),f)){
  char *p=line,*key,*value,*end;int index;
  if(++count>1024||(!strchr(line,'\n')&&!feof(f))){valid=false;break;}
  while(isspace((unsigned char)*p))++p;if(!*p||!strncmp(p,"//",2))continue;
  key=p;while(*p&&!isspace((unsigned char)*p))++p;if(!*p){valid=false;break;}*p++=0;
  while(isspace((unsigned char)*p))++p;value=p;
  if(*p=='"'){value=++p;end=strchr(p,'"');if(!end){valid=false;break;}*end++=0;while(isspace((unsigned char)*end))++end;if(*end){valid=false;break;}}
  else{end=p+strlen(p);while(end>p&&isspace((unsigned char)end[-1]))--end;*end=0;}
  index=Find(key);if(index<0||seen[index]||!ValidValue(index,value)){Con_Printf("GFX_INVALID name=%s line=%d\n",key,count);valid=false;break;}
  seen[index]=true;++total;if(InScope(index))strlcpy(pending[index],value,GFX_VALUE);
 }
 if(ferror(f))valid=false;fclose(f);return valid&&total>0;
}
static void PreviewFile(int index){
 if(index<0||index>=presetCount||index==previewIndex)return;
 previewIndex=index;Restore(baseline);preview=false;
 if(!ReadPreset(index)){strlcpy(notice,"Invalid or unsupported preset; original settings restored.",sizeof(notice));return;}
 Restore(pending);preview=true;snprintf(notice,sizeof(notice),"Enter/click: load; Esc: cancel | %s",files[index].name);
}
static void Apply(void){
 if(!browsing||!preview){strlcpy(notice,"No valid preview. Select a preset and press Enter to load it.",sizeof(notice));return;}
 if(previewIndex>=0)strlcpy(activeName,files[previewIndex].name,sizeof(activeName));else strlcpy(activeName,"Factory defaults",sizeof(activeName));Capture(applied);
 Capture(baseline);browsing=preview=typing=false;previewIndex=-1;snprintf(notice,sizeof(notice),"Loaded %s.%s",activeName,VideoPending()?" Restart video with F5.":"");
}
static void Cancel(void){Graphics_CancelPreview();}
static void SaveAs(void){typing=true;strlcpy(notice,"Type a new preset name. Enter saves; Escape cancels naming.",sizeof(notice));}
static void SaveNew(void){OpenPresets();SaveAs();}
static void SaveCurrent(void){char path[MAX_OSPATH];FILE *f;if(SafeName(activeName)){snprintf(path,sizeof(path),"%s/ezquake/presets/graphics/%s.cfg",com_basedir,activeName);f=fopen(path,"rb");if(f){fclose(f);scope=0;WritePreset(activeName,true);return;}}SaveNew();}
static void Reset(void){int i;Restore(baseline);for(i=0;i<optionCount;++i)if(InScope(i))Cvar_Set(options[i].row.cvar,options[i].row.cvar->defaultvalue);preview=true;previewIndex=-2;strlcpy(notice,"Factory defaults preview. F3: keep; Esc: cancel.",sizeof(notice));}
static qbool WritePreset(const char *name,qbool overwrite){
 char path[MAX_OSPATH],temp[MAX_OSPATH],backup[MAX_OSPATH];FILE *f;int i; qbool exists=false,ok;
 if(!SafeName(name)){strlcpy(notice,"Use 1-48 letters, digits, underscores or hyphens.",sizeof(notice));return false;}
 snprintf(path,sizeof(path),"%s/ezquake/presets/graphics/%s.cfg",com_basedir,name);
 f=fopen(path,"rb");if(f){fclose(f);exists=true;}
 if(exists&&!overwrite){strlcpy(notice,"Name already exists. Choose another name or use Save.",sizeof(notice));return false;}
 snprintf(temp,sizeof(temp),"%s.tmp",path);FS_CreatePath(temp);f=fopen(temp,"wb");if(!f){strlcpy(notice,"Cannot write preset directory.",sizeof(notice));return false;}
 fprintf(f,"// Graphics preset v1; category: %s\n",categories[scope]);
 for(i=0;i<optionCount;++i)if(InScope(i))fprintf(f,"%s \"%s\"\n",options[i].row.cvar->name,Value(options[i].row.cvar));
 ok=!ferror(f);if(fclose(f))ok=false;
 if(!ok){remove(temp);return false;}
 backup[0]=0;
 if(exists){for(i=1;i<10000;++i){snprintf(backup,sizeof(backup),"%s.bak-%d",path,i);f=fopen(backup,"rb");if(!f)break;fclose(f);}if(i==10000||rename(path,backup)){remove(temp);strlcpy(notice,"Cannot preserve the previous preset; save cancelled.",sizeof(notice));return false;}}
 if(rename(temp,path)){if(exists)rename(backup,path);strlcpy(notice,"Could not finish saving the preset.",sizeof(notice));return false;}
 strlcpy(activeName,name,sizeof(activeName));Capture(applied);Capture(baseline);preview=false;previewIndex=-1;typing=false;
 snprintf(notice,sizeof(notice),"Saved %s; previous version preserved when replacing a preset.",name);return true;
}
static void Save(void){
 if(previewIndex<0||files[previewIndex].readonly){SaveAs();return;}
 if(WritePreset(files[previewIndex].name,true)){browsing=false;}
}
static const char *ScopeName(void){return categories[scope];}
static void ScopeNext(qbool back){Restore(baseline);preview=false;previewIndex=-1;scope=(scope+(back?7:1))%8;strlcpy(notice,"Preset scope changed; previous graphics restored.",sizeof(notice));}
/* CFG-PRESETS-002: activation commits the focused row without navigating
 * through other live previews to reach a separate confirmation action. */
static void Pick(void){PreviewFile(presets.marked-presetStart);if(preview)Apply();}
static void Scan(const char *folder,qbool readonly,const char *prefix){
 dir_t dir;char path[MAX_OSPATH];int i;snprintf(path,sizeof(path),"%s/%s",com_basedir,folder);dir=Sys_listdir(path,"\\.cfg$",0);
 for(i=0;i<dir.numfiles&&presetCount<GFX_FILES;++i){char name[64];graphics_preset_t *f;if(dir.files[i].isdir)continue;strlcpy(name,dir.files[i].name,sizeof(name));if(strlen(name)<5)continue;name[strlen(name)-4]=0;if(!SafeName(name))continue;
  f=&files[presetCount++];strlcpy(f->name,name,sizeof(f->name));snprintf(f->label,sizeof(f->label),"%s%s",prefix,name);snprintf(f->path,sizeof(f->path),"%s/%s",path,dir.files[i].name);f->readonly=readonly;
 }
}
static void OpenPresets(void){
 int n=0,i;Ensure();if(browsing)return;Capture(baseline);browsing=true;preview=false;scope=0;previewIndex=-1;presetCount=0;
 Scan("ezquake/presets/graphics",false,"");Scan("ezquake/presets/graphics/builtin",true,"Built-in: ");Scan("ezquake/competitive",true,"Legacy: ");
 memset(presetRows,0,sizeof(presetRows));
 presetRows[n].type=stt_custom;presetRows[n].label="Preset scope";presetRows[n].readfnc=ScopeName;presetRows[n].togglefnc=ScopeNext;presetRows[n++].description="All graphics or one category. Other settings, keys, audio and match rules stay unchanged.";
 SetAction(&presetRows[n++],"Save as new preset",SaveAs,"Save current graphics using a new name. Existing files are preserved.");
 SetAction(&presetRows[n++],"Save selected user preset",Save,"Replace the selected user preset with a recoverable backup. Built-in and legacy files require Save As.");
 SetAction(&presetRows[n++],"Keep preview (F3)",Apply,"Press F3 anywhere in this browser to keep the current preview. Enter or click on a preset loads it directly. F5 restarts video afterwards if required.");
 SetAction(&presetRows[n++],"Cancel preview / Back",Cancel,"Restore every graphics value from before browsing, including pending video settings.");
 SetAction(&presetRows[n++],"Preview factory defaults",Reset,"Preview factory defaults for the selected category. F3 keeps them; Escape cancels.");
 presetRows[n].type=stt_separator;presetRows[n++].label="Highlight a preset to preview";presetStart=n;
 for(i=0;i<presetCount;++i)SetAction(&presetRows[n++],files[i].label,Pick,"Highlight to preview. Press Enter or click this preset to load it and return to Graphics. Escape cancels the preview. F5 restarts video after loading if required.");
 if(!presetCount)SetAction(&presetRows[n++],"No presets yet - Save As",SaveAs,"Create your first graphics preset from current settings.");
 presets.count=n;presets.marked=1;presets.viewpoint=0;Settings_OnShow(&presets);strlcpy(notice,"Highlight: preview | Enter/click: load | Esc: cancel",sizeof(notice));
}
static void IdentifyBuiltIn(void){int i,k;Capture(baseline);scope=0;presetCount=0;Scan("ezquake/presets/graphics/builtin",true,"Built-in: ");for(i=0;i<presetCount;++i){if(!ReadPreset(i))continue;for(k=0;k<optionCount;++k){char *a,*b;double av=strtod(baseline[k],&a),bv=strtod(pending[k],&b);if(a!=baseline[k]&&b!=pending[k]&&!*a&&!*b&&av==bv)continue;if(strcasecmp(baseline[k],pending[k]))break;}if(k==optionCount){strlcpy(activeName,files[i].name,sizeof(activeName));break;}}}
void Graphics_Draw(int x,int y,int w,int h){
 char status[256];Ensure();if(rebuild)BuildRows();
 if(browsing&&presets.marked>=presetStart&&presets.marked-presetStart<presetCount)PreviewFile(presets.marked-presetStart);
 snprintf(status,sizeof(status),"%s%s | %s%s",activeName,Changed(applied)?" * modified":"",browsing?"Preset preview":"Graphics",VideoPending()?" | Video restart required (F5)":"");
 if(VideoPending())snprintf(status,sizeof(status),"Video restart required (F5) | %s%s",activeName,Changed(applied)?" *":"");
 status[bound(1,w/8-1,(int)sizeof(status)-1)]=0;UI_Print(x,y,status,false);{char line[192];snprintf(line,sizeof(line),"%.*s",max(1,w/8-1),typing?va("Save as: %s_",filename):notice);UI_Print(x,y+12,line,false);}
 mouseTop=24;mouseLeft=0;Settings_Draw(x,y+24,w,h-24,browsing?&presets:&settings);
}
qbool Graphics_Key(int key,wchar unichar){
 qbool handled;Ensure();if(rebuild)BuildRows();
 if(typing){size_t n=strlen(filename);if(key==K_ESCAPE)typing=false;else if(key==K_ENTER){if(WritePreset(filename,false))browsing=false;}else if(key==K_BACKSPACE&&n)filename[n-1]=0;else if(key>0&&key<128&&n<48&&(isalnum(key)||key=='_'||key=='-')){filename[n]=(char)key;filename[n+1]=0;}return true;}
 if(key==K_F2){OpenPresets();return true;}if(key==K_F5){Restart();return true;}
 if(browsing&&key==K_F3){Apply();return true;}
 if(browsing&&(key==K_ESCAPE||key==K_MOUSE2)){Cancel();return true;}
 if(browsing&&key==K_TAB)return true;
 handled=Settings_Key(browsing?&presets:&settings,key,unichar);
 if(rebuild)BuildRows();
 if(browsing&&presets.marked>=presetStart)PreviewFile(presets.marked-presetStart);
 return handled;
}
qbool Graphics_Mouse(const mouse_state_t *ms){mouse_state_t m=*ms;qbool result;Ensure();if(typing)return true;m.y-=mouseTop;m.y_old-=mouseTop;m.x-=mouseLeft;result=Settings_Mouse_Event(browsing?&presets:&settings,&m);if(rebuild)BuildRows();if(browsing&&presets.marked>=presetStart)PreviewFile(presets.marked-presetStart);return result;}
void Graphics_OpenClarity(void){Ensure();category=6;BuildRows();Graphics_Open();}
static void Command(void){
 int i;const char *action=Cmd_Argv(1),*name=Cmd_Argv(2);Ensure();
 if(browsing){Con_Printf("gfx: Apply or Cancel the current preview first.\n");return;}
 if(!strcmp(action,"load")&&SafeName(name)){
  OpenPresets();for(i=0;i<presetCount;++i)if(!strcasecmp(files[i].name,name)){PreviewFile(i);break;}
  if(preview){Apply();Con_Printf("GFX_LOAD %s OK%s\n",name,VideoPending()?"; video restart required":"");}
  else{Cancel();Con_Printf("GFX_LOAD %s rejected or not found\n",name);}return;
 }
 if(!strcmp(action,"save")&&SafeName(name)){scope=0;WritePreset(name,false);Con_Printf("gfx: %s\n",notice);return;}
 if(!strcmp(action,"list")){OpenPresets();for(i=0;i<presetCount;++i)Con_Printf("%s\n",files[i].label);Cancel();return;}
 Con_Printf("gfx load <name> | save <new-name> | list. F2 in Graphics opens live previews.\n");
}
static void Developer(void){
 int i;const char *cmd=Cmd_Argv(1);Ensure();
 if(!strcmp(cmd,"select")){i=Find(Cmd_Argv(2));if(i>=0){category=options[i].category;BuildRows();for(i=0;i<settings.count;++i)if(settings.settings[i].cvar&&!strcmp(settings.settings[i].cvar->name,Cmd_Argv(2)))settings.marked=i;}}
 else if(!strcmp(cmd,"key"))Graphics_Key(Key_StringToKeynum(Cmd_Argv(2)),0);
 else if(!strcmp(cmd,"mouse")){mouse_state_t m={0};m.x=settings.width/2+16+(int)(UI_SliderWidth()*bound(0,atof(Cmd_Argv(2)),1));m.y=24+settings.settings[settings.marked].top-settings.settings[settings.viewpoint].top+4;m.x_old=m.x;m.y_old=m.y;Graphics_Mouse(&m);m.button_down=1;m.buttons[1]=true;Graphics_Mouse(&m);m.button_down=0;m.button_up=1;m.buttons[1]=false;Graphics_Mouse(&m);}
 else if(!strcmp(cmd,"presetclick")&&browsing){mouse_state_t m={0};m.x=presets.width/2;m.y=24+presets.settings[presets.marked].top-presets.settings[presets.viewpoint].top+4;m.x_old=m.x;m.y_old=m.y;Graphics_Mouse(&m);m.button_down=1;m.buttons[1]=true;Graphics_Mouse(&m);m.button_down=0;m.button_up=1;m.buttons[1]=false;Graphics_Mouse(&m);}
 else if(!strcmp(cmd,"browse"))OpenPresets();
 else if(!strcmp(cmd,"preview")){if(!browsing)OpenPresets();for(i=0;i<presetCount;++i)if(!strcmp(files[i].name,Cmd_Argv(2))){presets.marked=presetStart+i;PreviewFile(i);break;}}
 else if(!strcmp(cmd,"scope")){if(!browsing)OpenPresets();Restore(baseline);scope=bound(0,atoi(Cmd_Argv(2)),7);previewIndex=-1;preview=false;}
 else if(!strcmp(cmd,"apply"))Apply();else if(!strcmp(cmd,"cancel"))Cancel();
 else if(!strcmp(cmd,"saveas")){if(!browsing)OpenPresets();WritePreset(Cmd_Argv(2),false);}
 else if(!strcmp(cmd,"save"))Save();else if(!strcmp(cmd,"reset")){if(!browsing)OpenPresets();Reset();}
 else if(!strcmp(cmd,"savecurrent"))SaveCurrent();
 else if(!strcmp(cmd,"inventory")){for(i=0;i<optionCount;++i)Con_Printf("GFX_OPTION %d %s\n",options[i].category,options[i].row.cvar->name);}
 Con_Printf("GFX_STATE options=%d category=%d scope=%d browsing=%d preview=%d files=%d changed=%d restart=%d notice=%s\n",optionCount,category,scope,browsing,preview,presetCount,Changed(applied),VideoPending(),notice);
 if(settings.count&&settings.settings[settings.marked].cvar){setting *s=&settings.settings[settings.marked];Con_Printf("GFX_SELECTED %s enabled=%d value=%s\n",s->cvar->name,!Settings_Unavailable(s),Value(s->cvar));}
}
