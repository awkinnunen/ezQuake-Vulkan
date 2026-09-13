/* Competitive Visuals, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZ_COMPETITIVE_VISUALS_H
#define EZ_COMPETITIVE_VISUALS_H
#include "menu.h"
/* One registry drives cvars, bounded shader values, menu and visual-only profiles. */
#define CV_OPTIONS(X) \
 X(enable,"Enabled","0",0,1,1,1,"Enable world readability, styled lighting and silhouettes. General effects stay independent.") \
 X(detail,"Fine detail","1",0,1,.05,1,"Reduce fine texture grain while retaining broad patterns. One keeps original fine detail.") \
 X(pattern,"Pattern scale","2",0,5,.25,1,"Choose how broad the retained texture patterns are. Higher values use coarser mip levels.") \
 X(contrast,"Texture contrast","1",.2,1.5,.05,1,"Adjust fine-detail contrast relative to the broad texture pattern.") \
 X(saturation,"Saturation","1",0,1.5,.05,1,"Control world colour saturation. Zero is grey; one preserves original saturation.") \
 X(distance,"Distance smoothing","0",0,2,.1,1,"Gradually smooth distant texture detail to reduce visual noise.") \
 X(floor,"Floor colour","3",0,13,1,1,"Choose the Quake palette colour blended into floors by Surface tint.") \
 X(wall,"Wall colour","2",0,13,1,1,"Choose the Quake palette colour blended into walls by Surface tint.") \
 X(tint,"Surface tint","0",0,1,.05,1,"Blend floor and wall colours into textures. Zero preserves original material colours.") \
 X(light,"Light response","0",0,2,1,2,"Original lighting, smooth Gradient, or Soft Cel with adjustable lighting bands.") \
 X(lightmix,"Style blend","0",0,1,.05,2,"Blend the selected lighting style with original lighting. Zero removes the style.") \
 X(worldlight,"World lighting","1",0,1,1,2,"Apply the selected response to world lighting, including baked lightmaps.") \
 X(modellight,"Model lighting","1",0,1,1,2,"Apply the response to ordinary models. Server rules can restrict player styling.") \
 X(shadow,"Shadow level","0.2",0,.7,.025,2,"Set the brightness of the styled lighting shadow region. This is not a cast-shadow control.") \
 X(midtone,"Midtone level","0.55",.1,1,.025,2,"Set the brightness of the styled lighting middle region.") \
 X(highlight,"Highlight level","1",.5,1.5,.025,2,"Set the brightness of the styled lighting highlight region.") \
 X(bands,"Cel bands","4",2,8,1,2,"Set the number of Soft Cel lighting bands. Gradient mode does not use bands.") \
 X(softness,"Band softness","0.3",.05,1,.05,2,"Smooth transitions between Soft Cel lighting bands.") \
 X(warmth,"Warm / cool","0",-.2,.2,.025,2,"Shift styled highlights and shadows toward warm or cool colours.") \
 X(rim,"Player rim","0",0,.2,.01,3,"Light visible player surfaces near the silhouette. Suppressed on powerups and by rulesets.") \
 X(rimwidth,"Rim width","3",1,8,.25,3,"Shape the player rim. Higher values produce a narrower rim.") \
 X(rimup,"Upward bias","0.65",0,1,.05,3,"Emphasize upward-facing player surfaces and reduce the rim on undersides.") \
 X(rimteam,"Team tint","0.15",0,.3,.025,3,"Blend a subtle shirt-colour tint into the player rim.") \
 X(edgewidth,"World edge width","1",.5,3,.1,3,"Control screen-space world line width when world outlines are enabled.") \
 X(edgestrength,"World edge opacity","1",0,1,.05,3,"Control world outline opacity. Zero makes world lines invisible.") \
 X(modelopacity,"Model edge opacity","1",0,1,.05,3,"Control model outline opacity. Requires model outlines and nonzero model edge width.") \
 X(edgepalette,"Use edge palette","0",0,1,1,3,"Replace outline colours with the world, friend and enemy palette choices below.") \
 X(worldedge,"World edge colour","0",0,13,1,3,"Set the palette colour used for world geometry outlines.") \
 X(teamedge,"Friend edge colour","3",0,13,1,3,"Set the palette colour used for friendly player outlines.") \
 X(enemyedge,"Enemy edge colour","0",0,13,1,3,"Set the palette colour used for enemy players and other model outlines.") \
 X(bloom,"Bloom strength","0",0,.15,.005,4,"Add a glow around bright scene pixels. Zero disables bloom. The HUD is excluded.") \
 X(bloomthreshold,"Bloom threshold","0.9",.5,2,.05,4,"Set the brightness required for pixels to contribute to bloom.") \
 X(bloomradius,"Bloom radius","2",.5,5,.25,4,"Set the small glow radius in screen pixels. Requires nonzero Bloom strength.") \
 X(ao,"World AO strength","0",0,.4,.025,4,"Darken world surface contacts using depth and normals. Zero disables contact shading.") \
 X(aoradius,"World AO radius","12",2,32,1,4,"Set the world-space contact shading sample radius. Requires nonzero World AO strength.") \
 X(exposure,"Exposure","1",.5,2,.05,5,"Adjust scene exposure before the HUD. One preserves the original exposure.") \
 X(tonemap,"Tone curve","0",0,2,1,5,"Original response, a soft highlight shoulder, or a filmic tone curve. HUD is excluded.") \
 X(sharpen,"Scene sharpness","0",0,.5,.025,5,"Sharpen scene detail after rendering. Zero disables sharpening. HUD is excluded.")
#define CV_ENUM(id,label,def,lo,hi,step,page,help) CV_##id,
typedef enum { CV_OPTIONS(CV_ENUM) CV_COUNT } cv_id;
#undef CV_ENUM
typedef struct { float world[4], floor[4], wall[4], light[4], levels[4], shape[4], rim[4], rimcolor[4], post[4], post2[4], modelview[16]; } cv_params_t;
void CV_Init(void);
float CV_Value(cv_id id);
qbool CV_Active(void);
qbool CV_PostActive(void);
void CV_VideoApplied(int requested, int actual);
void CV_TestView(void);
qbool CV_RimAllowed(int effects, qbool player, qbool weapon, qbool blocked);
void CV_Params(cv_params_t *out);
void CV_EdgeColor(float *out, int kind);
void CV_PlayerParams(cv_params_t *out, entity_t *ent, int effects, int render_effects, const float *modelview);
void CV_Open(void);
void CV_OpenEffects(void);
void CV_Draw(void);
void CV_Key(int key);
qbool CV_Mouse(const mouse_state_t *ms);
#endif
