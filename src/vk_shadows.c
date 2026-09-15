/* SHADOW-001, OpenAI Codex, 2026-09-14. GPL-2.0-or-later.
 * SHADOW-002, OpenAI Codex: cached point/spot maps, BSP lights and per-view resources.
 */
#ifdef RENDERER_OPTION_VULKAN
#include <vulkan/vulkan.h>
#include "quakedef.h"
#include "gl_model.h"
#include "r_buffers.h"
#include "r_matrix.h"
#include "r_local.h"
#include "r_lighting.h"
#include "r_texture.h"
#include "vk_local.h"
#include "competitive_visuals.h"
#include "vk_shadows.h"
#include <stddef.h>
#include <math.h>

#define SHADOW_LIGHTS 8
#define SHADOW_VIEWS 4
#define SHADOW_CASTERS 8192
#define SHADOW_ALIAS 256
#define SHADOW_WIDTH 3072
#define SHADOW_HEIGHT 4096
typedef struct {float inverse[16],viewport[4],settings[4],positions[SHADOW_LIGHTS][4],colors[SHADOW_LIGHTS][4],camera[4],directions[SHADOW_LIGHTS][4],modes[4];} shadow_uniform_t;
typedef struct {float world[16],light[4],params[4],direction[4];} shadow_push_t;
typedef char shadow_uniform_size[(sizeof(shadow_uniform_t)==512)?1:-1];
typedef char shadow_push_size[(sizeof(shadow_push_t)==112)?1:-1];
typedef struct {unsigned first,count;float modelview[16],lerp,radius;texture_ref texture;} shadow_alias_t;
typedef struct {unsigned first,count;float world[16];texture_ref texture; qbool alias;float lerp;float center[3],radius;} shadow_caster_t;
static struct {VkImage image;VkDeviceMemory memory;VkImageView view;VkFramebuffer framebuffer;
 VkBuffer buffer;VkDeviceMemory bufferMemory;shadow_uniform_t *mapped;qbool initialized; unsigned width,height; unsigned long long hashes[SHADOW_LIGHTS]; unsigned ages[SHADOW_LIGHTS];} frames[VK_MAX_FRAMES_IN_FLIGHT*SHADOW_VIEWS];
static int viewIndex, nextView;
int VK_ShadowViewIndex(void){return viewIndex;}
static int Slot(void){return vk_options.frame.currentFrame*SHADOW_VIEWS+viewIndex;}
static VkRenderPass pass;
static VkSampler sampler;
static VkPipelineLayout pipelineLayout;
static VkPipeline pipelines[2];
static VkFormat format;
#define MAP_LIGHTS 256
#define LIGHT_CANDIDATES (MAX_DLIGHTS+MAP_LIGHTS)
typedef struct {dlight_t light;float direction[3],cone;int style;} map_light_t;
static map_light_t mapLights[MAP_LIGHTS],lightData[LIGHT_CANDIDATES];
static int mapLightCount;static model_t *lightWorld;
static unsigned updateCursor[VK_MAX_FRAMES_IN_FLIGHT*SHADOW_VIEWS];
static int selected[SHADOW_LIGHTS],selectedCount,selectedHistory[SHADOW_VIEWS][SHADOW_LIGHTS];
static unsigned lastCached,lastCulled,lastDeferred;
static int viewLights[SHADOW_VIEWS];
static shadow_caster_t *worldCasters; static int worldCasterCount; static model_t *casterWorld;
static shadow_alias_t aliases[SHADOW_ALIAS];
static int aliasCount;
static qbool aliasOverflow;
static shadow_caster_t casters[SHADOW_CASTERS];
static int casterCount;
static unsigned lastDraws,lastFaces,lastOverflow;
static model_t *previousWorld;
static double previousTime;
extern int dlightcolor[NUM_DLIGHTTYPES][3];
extern cvar_t r_dynamic,gl_flashblend,cl_multiview,r_fullbright;
extern const unsigned char vk_shadow_vert_spv[],vk_shadow_frag_spv[];
extern const unsigned int vk_shadow_vert_spv_len,vk_shadow_frag_spv_len;

static void Check(VkResult result,const char *where){if(result!=VK_SUCCESS) Sys_Error("Vulkan shadow %s failed: %d",where,result);}
static VkShaderModule Module(const unsigned char *data,unsigned size) {
 VkShaderModule module;VkShaderModuleCreateInfo info={0};void *aligned=Q_malloc(size);
 memcpy(aligned,data,size);info.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;info.codeSize=size;info.pCode=aligned;
 Check(vkCreateShaderModule(vk_options.logicalDevice,&info,NULL,&module),"shader");Q_free(aligned);return module;
}
static void CreatePass(void) {
 VkAttachmentDescription a={0};VkAttachmentReference ref={0,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
 VkSubpassDescription sub={0};VkSubpassDependency deps[2]={{0}};VkRenderPassCreateInfo info={0};VkSamplerCreateInfo samp={0};
 VkFormatProperties props;
 if(pass) return;
 format=VK_FORMAT_D32_SFLOAT;vkGetPhysicalDeviceFormatProperties(vk_options.physicalDevice,format,&props);
 if((props.optimalTilingFeatures&(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))!=(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) format=VK_FORMAT_D16_UNORM;
 a.format=format;a.samples=VK_SAMPLE_COUNT_1_BIT;a.loadOp=VK_ATTACHMENT_LOAD_OP_LOAD;a.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
 a.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;a.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
 a.initialLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;a.finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;sub.pDepthStencilAttachment=&ref;
 deps[0].srcSubpass=VK_SUBPASS_EXTERNAL;deps[0].dstSubpass=0;
 deps[0].srcStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;deps[0].srcAccessMask=VK_ACCESS_SHADER_READ_BIT;
 deps[0].dstStageMask=VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT|VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;deps[0].dstAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
 deps[1].srcSubpass=0;deps[1].dstSubpass=VK_SUBPASS_EXTERNAL;
 deps[1].srcStageMask=deps[0].dstStageMask;deps[1].srcAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
 deps[1].dstStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;deps[1].dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
 info.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;info.attachmentCount=1;info.pAttachments=&a;
 info.subpassCount=1;info.pSubpasses=&sub;info.dependencyCount=2;info.pDependencies=deps;
 Check(vkCreateRenderPass(vk_options.logicalDevice,&info,NULL,&pass),"render pass");
 samp.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;samp.magFilter=samp.minFilter=VK_FILTER_NEAREST;
 samp.addressModeU=samp.addressModeV=samp.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
 Check(vkCreateSampler(vk_options.logicalDevice,&samp,NULL,&sampler),"sampler");
}
static void DestroyFrame(int i) {
 if(frames[i].mapped) vkUnmapMemory(vk_options.logicalDevice,frames[i].bufferMemory);
 if(frames[i].buffer) vkDestroyBuffer(vk_options.logicalDevice,frames[i].buffer,NULL);
 if(frames[i].bufferMemory) vkFreeMemory(vk_options.logicalDevice,frames[i].bufferMemory,NULL);
 if(frames[i].framebuffer) vkDestroyFramebuffer(vk_options.logicalDevice,frames[i].framebuffer,NULL);
 if(frames[i].view) vkDestroyImageView(vk_options.logicalDevice,frames[i].view,NULL);
 if(frames[i].image) vkDestroyImage(vk_options.logicalDevice,frames[i].image,NULL);
 if(frames[i].memory) vkFreeMemory(vk_options.logicalDevice,frames[i].memory,NULL);
 memset(&frames[i],0,sizeof(frames[i]));
}
void VK_ShadowFrameResources(void) {
 int v; unsigned resolution=(unsigned)CV_Value(CV_shadowquality),width=VK_ShadowEnabled()?resolution*6:1,height=VK_ShadowEnabled()?resolution*8:1;
 CreatePass(); viewIndex=nextView=0;selectedCount=0;memset(viewLights,0,sizeof(viewLights));
 for(v=0;v<SHADOW_VIEWS;++v) {
  int i=vk_options.frame.currentFrame*SHADOW_VIEWS+v;
  unsigned w=(v==0 || v<cl_multiview.integer)?width:1,h=(v==0 || v<cl_multiview.integer)?height:1;
  VkImageViewCreateInfo view={0};VkFramebufferCreateInfo fb={0};
  if(frames[i].image && frames[i].width!=w) DestroyFrame(i);
  if(!frames[i].image) {
   frames[i].width=w;frames[i].height=h;
   if(!VK_CreateImageResource(w,h,1,VK_SAMPLE_COUNT_1_BIT,format,VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,&frames[i].image,&frames[i].memory)) Sys_Error("Vulkan: shadow atlas allocation failed");
   view.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;view.image=frames[i].image;view.viewType=VK_IMAGE_VIEW_TYPE_2D;view.format=format;
   view.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;view.subresourceRange.levelCount=view.subresourceRange.layerCount=1;
   Check(vkCreateImageView(vk_options.logicalDevice,&view,NULL,&frames[i].view),"image view");
   fb.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;fb.renderPass=pass;fb.attachmentCount=1;fb.pAttachments=&frames[i].view;
   fb.width=w;fb.height=h;fb.layers=1;
   Check(vkCreateFramebuffer(vk_options.logicalDevice,&fb,NULL,&frames[i].framebuffer),"framebuffer");
   if(!VK_CreateBufferResource(sizeof(shadow_uniform_t),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&frames[i].buffer,&frames[i].bufferMemory)) Sys_Error("Vulkan: shadow uniform allocation failed");
   Check(vkMapMemory(vk_options.logicalDevice,frames[i].bufferMemory,0,VK_WHOLE_SIZE,0,(void**)&frames[i].mapped),"uniform map");
  }
  memset(frames[i].mapped,0,sizeof(shadow_uniform_t));
 }
}
void VK_ShadowDescriptors(int v,VkDescriptorImageInfo *image,VkDescriptorBufferInfo *buffer) {
 int i=vk_options.frame.currentFrame*SHADOW_VIEWS+v;
 image->sampler=sampler;image->imageView=frames[i].view;image->imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 buffer->buffer=frames[i].buffer;buffer->offset=0;buffer->range=sizeof(shadow_uniform_t);
}
qbool VK_ShadowEnabled(void){return R_UseVulkan()&&CV_Value(CV_shadows)>0&&r_dynamic.value>0&&!gl_flashblend.integer&&!r_fullbright.value;}
// Read existing BSP light entities and optional additive lights/<map>.vklights.
// The optional file uses the same quoted key/value entity syntax; no asset changes.
typedef struct {char classname[64],target[64],name[64];vec3_t origin,color;float radius,cone;int style;qbool hasOrigin;} light_entity_t;
static void ParseLights(const char *data) {
 light_entity_t *entities=Q_calloc(4096,sizeof(*entities));int count=0,i,j;
 while(data && (data=COM_Parse(data)) && count<4096) {
  light_entity_t e={0};char key[128];e.radius=300;e.cone=40;VectorSet(e.color,1,1,1);
  if(strcmp(com_token,"{"))break;
  while((data=COM_Parse(data)) && strcmp(com_token,"}")) {
   strlcpy(key,com_token,sizeof(key));data=COM_Parse(data);if(!data)break;
   if(!strcmp(key,"classname"))strlcpy(e.classname,com_token,sizeof(e.classname));
   else if(!strcmp(key,"origin"))e.hasOrigin=sscanf(com_token,"%f %f %f",&e.origin[0],&e.origin[1],&e.origin[2])==3;
   else if(!strcmp(key,"target"))strlcpy(e.target,com_token,sizeof(e.target));
   else if(!strcmp(key,"targetname"))strlcpy(e.name,com_token,sizeof(e.name));
   else if(!strcmp(key,"style"))e.style=bound(0,atoi(com_token),255);
   else if(!strcmp(key,"light"))e.radius=(float)atof(com_token);
   else if(!strcmp(key,"_light")){float a,b,c,d;if(sscanf(com_token,"%f %f %f %f",&a,&b,&c,&d)==4){VectorSet(e.color,a/255,b/255,c/255);e.radius=d;}else e.radius=(float)atof(com_token);}
   else if(!strcmp(key,"_color"))sscanf(com_token,"%f %f %f",&e.color[0],&e.color[1],&e.color[2]);
   else if(!strcmp(key,"_cone"))e.cone=bound(5,(float)atof(com_token),160);
  }
  entities[count++]=e;
 }
 for(i=0;i<count && mapLightCount<MAP_LIGHTS;++i) {
  light_entity_t *e=&entities[i];map_light_t *l;
  if(strncmp(e->classname,"light",5) || !e->hasOrigin || e->radius<16)continue;
  l=&mapLights[mapLightCount++];memset(l,0,sizeof(*l));l->light.type=lt_custom;l->style=e->style;
  VectorCopy(e->origin,l->light.origin);l->light.radius=bound(16,e->radius,4096);
  for(j=0;j<3;++j)l->light.color[j]=(int)(bound(0,e->color[j],4)*255);
  if(e->target[0])for(j=0;j<count;++j)if(!strcmp(e->target,entities[j].name)&&entities[j].hasOrigin){
   VectorSubtract(entities[j].origin,e->origin,l->direction);
   if(VectorNormalize(l->direction)>0)l->cone=cosf(e->cone*(float)M_PI/360);break;
  }
 }
 Q_free(entities);
}
static void MapLights(void) {
 if(lightWorld!=cl.worldmodel) {
  char path[MAX_QPATH],map[MAX_QPATH];byte *data;int length;lightWorld=cl.worldmodel;mapLightCount=0;
  if(!lightWorld)return;
  COM_FileBase(lightWorld->name,map);
  data=FS_LoadHeapFile(lightWorld->name,&length);
  if(data && length>=sizeof(dheader_t)) {
   dheader_t *header=(dheader_t*)data;
   int offset=LittleLong(header->lumps[LUMP_ENTITIES].fileofs),size=LittleLong(header->lumps[LUMP_ENTITIES].filelen);
   if(offset>=0 && size>=0 && offset<=length && size<=length-offset) {
    char *entities=Q_malloc((size_t)size+1);memcpy(entities,data+offset,size);entities[size]=0;
    ParseLights(entities);Q_free(entities);
   }
  }
  Q_free(data);
  snprintf(path,sizeof(path),"lights/%s.vklights",map);
  data=FS_LoadHeapFile(path,NULL);if(data){ParseLights((const char*)data);Q_free(data);}
  Con_Printf("SHADOW_MAP lights=%d map=%s\n",mapLightCount,map);
 }
}
void VK_ShadowSelect(void) {
 int old[SHADOW_LIGHTS],i,k;
 viewIndex=bound(0,nextView,SHADOW_VIEWS-1);++nextView;float scores[LIGHT_CANDIDATES]={0};
 memcpy(old,selectedHistory[viewIndex],sizeof(old));
 CV_TestLights();MapLights();
 memset(lightData,0,sizeof(lightData));
 for(i=0;i<MAX_DLIGHTS;++i)lightData[i].light=cl_dlights[i];
 for(i=0;i<mapLightCount;++i){lightData[MAX_DLIGHTS+i]=mapLights[i];lightData[MAX_DLIGHTS+i].light.radius*=CV_Value(CV_maplightscale);}
 selectedCount=aliasCount=0;aliasOverflow=false;memset(selected,255,sizeof(selected));
 if(previousWorld!=cl.worldmodel || cl.time<previousTime || cl.time-previousTime>1) {memset(old,255,sizeof(old));memset(selectedHistory,255,sizeof(selectedHistory));}
 previousWorld=cl.worldmodel;previousTime=cl.time;
 if(!VK_ShadowEnabled()||!frames[Slot()].mapped) return;
 for(i=0;i<MAX_DLIGHTS+(CV_Value(CV_maplights)>0?mapLightCount:0);++i) {
  vec3_t d;float distance;
  if((i<MAX_DLIGHTS && !(cl_dlight_active[i/32]&(1u<<(i%32)))) || lightData[i].light.radius<16) continue;
  VectorSubtract(lightData[i].light.origin,r_refdef.vieworg,d);distance=VectorLength(d);
  if(distance>CV_Value(CV_shadowdistance)+lightData[i].light.radius) continue;
  scores[i]=lightData[i].light.radius/(distance+64);
  for(k=0;k<SHADOW_LIGHTS;++k) if(i==old[k]) {scores[i]*=1.15f;break;}
 }
 for(k=0;k<bound(1,(int)CV_Value(CV_shadowlights),SHADOW_LIGHTS);++k) {
  int best=-1;float score=0;
  for(i=0;i<LIGHT_CANDIDATES;++i) if(scores[i]>score){best=i;score=scores[i];}
  if(best<0) break;
  selected[selectedCount++]=best;scores[best]=0;
 }
 memcpy(selectedHistory[viewIndex],selected,sizeof(selected));viewLights[viewIndex]=selectedCount;
}
qbool VK_ShadowManagedLight(int index) {int i;if(!R_UseVulkan()) return false;for(i=0;i<selectedCount;++i) if(index==selected[i]) return true;return false;}
qbool VK_ShadowCasterRelevant(entity_t *ent) {
 int i;if(!R_UseVulkan()||!ent||ent->renderfx&RF_WEAPONMODEL) return false;
 for(i=0;i<selectedCount;++i){vec3_t d;VectorSubtract(ent->origin,lightData[selected[i]].light.origin,d);if(VectorLength(d)<lightData[selected[i]].light.radius+(ent->model?ent->model->radius:128)) return true;}
 return false;
}
void VK_ShadowAddAlias(unsigned first,unsigned count,const float *mv,float lerp,texture_ref texture,float radius) {
 shadow_alias_t *a;if(!selectedCount) return;if(aliasCount==SHADOW_ALIAS){aliasOverflow=true;return;}
 a=&aliases[aliasCount++];a->first=first;a->count=count;a->lerp=lerp;a->radius=radius;a->texture=texture;memcpy(a->modelview,mv,64);
}
static void WorldMatrix(const float *mv,float *world) {
 float view[16];int axis,col,k;R_GetModelviewMatrix(view);R_SetIdentityMatrix(world);
 for(axis=0;axis<3;++axis) for(col=0;col<4;++col) {
  float value=col==3?r_refdef.vieworg[axis]:0;
  for(k=0;k<3;++k) value+=view[axis*4+k]*mv[col*4+k];
  world[col*4+axis]=value;
 }
}
static qbool Inverse(const float *m,float *result) {
 double a[4][8];int r,c,k;
 for(r=0;r<4;++r) for(c=0;c<8;++c) a[r][c]=c<4?m[c*4+r]:(c-4==r);
 for(c=0;c<4;++c) {
  int pivot=c;double value;
  for(r=c+1;r<4;++r) if(fabs(a[r][c])>fabs(a[pivot][c])) pivot=r;
  if(fabs(a[pivot][c])<1e-12) return false;
  for(k=0;k<8;++k){double t=a[c][k];a[c][k]=a[pivot][k];a[pivot][k]=t;}
  value=a[c][c];for(k=0;k<8;++k) a[c][k]/=value;
  for(r=0;r<4;++r) if(r!=c){value=a[r][c];for(k=0;k<8;++k) a[r][k]-=value*a[c][k];}
 }
 for(r=0;r<4;++r) for(c=0;c<4;++c) result[c*4+r]=(float)a[r][c+4];return true;
}
static VkPipeline Pipeline(qbool alias) {
 VkGraphicsPipelineCreateInfo info={0};VkPipelineShaderStageCreateInfo stages[2]={{0}};
 VkVertexInputBindingDescription binding={0};VkVertexInputAttributeDescription attr[3]={{0}};
 VkPipelineVertexInputStateCreateInfo vertex={0};VkPipelineInputAssemblyStateCreateInfo assembly={0};
 VkPipelineViewportStateCreateInfo viewport={0};VkPipelineRasterizationStateCreateInfo raster={0};
 VkPipelineMultisampleStateCreateInfo ms={0};VkPipelineDepthStencilStateCreateInfo depth={0};
 VkPipelineColorBlendStateCreateInfo blend={0};VkPipelineDynamicStateCreateInfo dynamic={0};
 VkDynamicState states[2]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};int i;
 if(pipelines[alias]) return pipelines[alias];
 if(!pipelineLayout) {
  VkDescriptorSetLayout texture=VK_TextureDescriptorSetLayout();VkPushConstantRange range={VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(shadow_push_t)};
  VkPipelineLayoutCreateInfo layout={0};layout.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;layout.setLayoutCount=1;layout.pSetLayouts=&texture;
  layout.pushConstantRangeCount=1;layout.pPushConstantRanges=&range;
  Check(vkCreatePipelineLayout(vk_options.logicalDevice,&layout,NULL,&pipelineLayout),"pipeline layout");
 }
 for(i=0;i<2;++i){stages[i].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;stages[i].stage=i?VK_SHADER_STAGE_FRAGMENT_BIT:VK_SHADER_STAGE_VERTEX_BIT;stages[i].pName="main";}
 stages[0].module=Module(vk_shadow_vert_spv,vk_shadow_vert_spv_len);stages[1].module=Module(vk_shadow_frag_spv,vk_shadow_frag_spv_len);
 binding.stride=alias?sizeof(vbo_model_vert_t):sizeof(vbo_world_vert_t);binding.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;
 for(i=0;i<3;++i){attr[i].location=i;attr[i].format=i==1?VK_FORMAT_R32G32_SFLOAT:VK_FORMAT_R32G32B32_SFLOAT;}
 attr[1].offset=alias?offsetof(vbo_model_vert_t,texture_coords):offsetof(vbo_world_vert_t,material_coords);
 attr[2].offset=alias?offsetof(vbo_model_vert_t,direction):offsetof(vbo_world_vert_t,position);
 vertex.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;vertex.vertexBindingDescriptionCount=1;vertex.pVertexBindingDescriptions=&binding;vertex.vertexAttributeDescriptionCount=3;vertex.pVertexAttributeDescriptions=attr;
 assembly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;assembly.topology=alias?VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
 viewport.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;viewport.viewportCount=viewport.scissorCount=1;
 raster.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;raster.polygonMode=VK_POLYGON_MODE_FILL;raster.cullMode=VK_CULL_MODE_NONE;raster.lineWidth=1;
 ms.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
 depth.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;depth.depthTestEnable=depth.depthWriteEnable=VK_TRUE;depth.depthCompareOp=VK_COMPARE_OP_LESS_OR_EQUAL;
 blend.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
 dynamic.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;dynamic.dynamicStateCount=2;dynamic.pDynamicStates=states;
 info.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;info.stageCount=2;info.pStages=stages;info.pVertexInputState=&vertex;info.pInputAssemblyState=&assembly;info.pViewportState=&viewport;
 info.pRasterizationState=&raster;info.pMultisampleState=&ms;info.pDepthStencilState=&depth;info.pColorBlendState=&blend;info.pDynamicState=&dynamic;info.layout=pipelineLayout;info.renderPass=pass;
 Check(vkCreateGraphicsPipelines(vk_options.logicalDevice,vk_options.pipelineCache,1,&info,NULL,&pipelines[alias]),"pipeline");
 for(i=0;i<2;++i) vkDestroyShaderModule(vk_options.logicalDevice,stages[i].module,NULL);
 return pipelines[alias];
}
static qbool AddModel(model_t *model,const float *world,const vec3_t light,float radius) {
 int s,v,axis;glpoly_t *poly;
 if(!model||model->type!=mod_brush) return true;
 for(s=0;s<model->nummodelsurfaces;++s) {
  msurface_t *surface=&model->surfaces[model->firstmodelsurface+s];texture_t *tex=surface->texinfo->texture;
  if(surface->flags&(SURF_DRAWSKY|SURF_DRAWTURB)) continue;
  for(poly=surface->polys;poly;poly=poly->next) {
   vec3_t mins={1e30f,1e30f,1e30f},maxs={-1e30f,-1e30f,-1e30f};float distance=0;
   shadow_caster_t *c;
   if(poly->numverts<3) continue;
   for(v=0;v<poly->numverts;++v) for(axis=0;axis<3;++axis) {
    float p=world[12+axis]+world[axis]*poly->verts[v][0]+world[4+axis]*poly->verts[v][1]+world[8+axis]*poly->verts[v][2];
    mins[axis]=min(mins[axis],p);maxs[axis]=max(maxs[axis],p);
   }
   for(axis=0;axis<3;++axis){float d=max(max(mins[axis]-light[axis],light[axis]-maxs[axis]),0);distance+=d*d;}
   if(distance>radius*radius) continue;
   if(casterCount>=bound(64,(int)CV_Value(CV_shadowcasters),SHADOW_CASTERS)) return false;
   c=&casters[casterCount++];memset(c,0,sizeof(*c));memcpy(c->world,world,64);
   for(axis=0;axis<3;++axis) c->center[axis]=(mins[axis]+maxs[axis])*.5f;
   {vec3_t ext;VectorSubtract(maxs,c->center,ext);c->radius=VectorLength(ext);}
   c->first=poly->vbo_start;c->count=poly->numverts;c->texture=tex->gl_texturenum;
   if(!VK_TextureReady(c->texture)) c->texture=solidwhite_texture;
  }
 }
 return true;
}
static qbool GatherCasters(int lightIndex) {
 float identity[16];int i;dlight_t *light=&lightData[lightIndex].light;
 /* SHADOW-003: static world occlusion is already baked into mode 1 lightmaps.
  * Reapplying it from a camera-selected subset of lights darkens whole rooms
  * as that subset changes. Only additional entity occluders belong in this
  * mode. Dynamic lights and realtime replacement still need the full world. */
 qbool bakedMapLight=lightIndex>=MAX_DLIGHTS && (int)CV_Value(CV_maplights)==1;
 casterCount=0;if(aliasOverflow) return false;
 R_SetIdentityMatrix(identity);
 if(casterWorld!=cl.worldmodel) {
  Q_free(worldCasters);worldCasters=NULL;worldCasterCount=0;casterWorld=cl.worldmodel;
  /* Cache world-space polygon bounds once per map, independently of light budget. */
  {int s; for(s=0;s<cl.worldmodel->nummodelsurfaces;++s) {
   msurface_t *surf=&cl.worldmodel->surfaces[cl.worldmodel->firstmodelsurface+s];glpoly_t *p;
   if(surf->flags&(SURF_DRAWSKY|SURF_DRAWTURB)) continue;
   for(p=surf->polys;p;p=p->next) if(p->numverts>=3) ++worldCasterCount;
  }}
  worldCasters=Q_calloc(worldCasterCount,sizeof(*worldCasters));
  {int s,n=0; for(s=0;s<cl.worldmodel->nummodelsurfaces;++s) {
   msurface_t *surf=&cl.worldmodel->surfaces[cl.worldmodel->firstmodelsurface+s];glpoly_t *p;
   if(surf->flags&(SURF_DRAWSKY|SURF_DRAWTURB)) continue;
   for(p=surf->polys;p;p=p->next) if(p->numverts>=3) {
    int a,v; vec3_t mn={1e30f,1e30f,1e30f},mx={-1e30f,-1e30f,-1e30f},ext;shadow_caster_t *c=&worldCasters[n++];
    c->first=p->vbo_start;c->count=p->numverts;c->texture=surf->texinfo->texture->gl_texturenum;memcpy(c->world,identity,64);
    for(v=0;v<p->numverts;++v) for(a=0;a<3;++a){mn[a]=min(mn[a],p->verts[v][a]);mx[a]=max(mx[a],p->verts[v][a]);}
    for(a=0;a<3;++a)c->center[a]=(mn[a]+mx[a])*.5f;
    VectorSubtract(mx,c->center,ext);c->radius=VectorLength(ext);
   }
  }}
 }
 for(i=0;!bakedMapLight && i<worldCasterCount;++i) {
  vec3_t d;shadow_caster_t *c=&worldCasters[i];VectorSubtract(c->center,light->origin,d);
  if(DotProduct(d,d)>(light->radius+c->radius)*(light->radius+c->radius))continue;
  if(casterCount>=bound(64,(int)CV_Value(CV_shadowcasters),SHADOW_CASTERS))return false;
  casters[casterCount++]=*c;
 }
 for(i=0;i<cl_visents.count;++i) {
  entity_t *ent=&cl_visents.list[i].ent;float old[16],mv[16],world[16];
  if(!ent->model||ent->model->type!=mod_brush||(ent->alpha>0&&ent->alpha<1)) continue;
  R_PushModelviewMatrix(old);R_RotateForEntity(ent);R_GetModelviewMatrix(mv);R_PopModelviewMatrix(old);WorldMatrix(mv,world);
  if(!AddModel(ent->model,world,light->origin,light->radius)) return false;
 }
 for(i=0;i<aliasCount;++i) {
  shadow_caster_t candidate={0},*c=&candidate;vec3_t distance;
  WorldMatrix(aliases[i].modelview,c->world);
  VectorCopy(c->world+12,c->center);c->radius=aliases[i].radius;
  VectorSubtract(c->center,light->origin,distance);
  if(DotProduct(distance,distance)>(light->radius+c->radius)*(light->radius+c->radius))continue;
  if(casterCount>=bound(64,(int)CV_Value(CV_shadowcasters),SHADOW_CASTERS)) return false;
  c->alias=true;c->first=aliases[i].first;c->count=aliases[i].count;c->texture=aliases[i].texture;c->lerp=aliases[i].lerp;
  casters[casterCount++]=candidate;
 }
 return true;
}
qbool VK_ShadowWorkPending(void) {return VK_ShadowEnabled() && (selectedCount>0 || CV_Value(CV_maplights)>1);}
void VK_ShadowBeginCommands(void) {
 int v;VkCommandBuffer command=VK_CurrentCommandBuffer();
 lastDraws=lastFaces=lastOverflow=lastCached=lastCulled=lastDeferred=0;
 for(v=0;v<SHADOW_VIEWS;++v) {
  int slot=vk_options.frame.currentFrame*SHADOW_VIEWS+v;
  if(command && frames[slot].mapped && !frames[slot].initialized) {
   VkImageMemoryBarrier b={0};VkClearDepthStencilValue clear={1,0};
   b.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;b.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;b.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   b.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
   b.image=frames[slot].image;b.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;b.subresourceRange.levelCount=b.subresourceRange.layerCount=1;
   vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,NULL,0,NULL,1,&b);
   vkCmdClearDepthStencilImage(command,b.image,b.newLayout,&clear,1,&b.subresourceRange);
   b.oldLayout=b.newLayout;b.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;b.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;b.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
   vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,NULL,0,NULL,1,&b);
   frames[slot].initialized=true;
  }
 }
}
static unsigned long long Hash(const void *p,size_t size,unsigned long long h) {
 const byte *b=p;while(size--) {h^=*b++;h*=1099511628211ULL;}return h;
}
static qbool FaceRelevant(const shadow_caster_t *c,const float *light,int face) {
 float d[3],x,y,z,r=c->radius;VectorSubtract(c->center,light,d);
 switch(face){case 0:x=-d[2];y=-d[1];z=d[0];break;case 1:x=d[2];y=-d[1];z=-d[0];break;
 case 2:x=d[0];y=d[2];z=d[1];break;case 3:x=d[0];y=-d[2];z=-d[1];break;
 case 4:x=d[0];y=-d[1];z=d[2];break;default:x=-d[0];y=-d[1];z=-d[2];break;}
 return z+r>=1 && fabsf(x)-z<=r*1.414214f && fabsf(y)-z<=r*1.414214f;
}
static qbool SpotRelevant(const shadow_caster_t *c,const float *light,const float *direction) {
 vec3_t d,up={0,0,1},right;float x,y,z,scale,r=c->radius;
 if(fabsf(direction[2])>.99f)VectorSet(up,0,1,0);
 CrossProduct(direction,up,right);VectorNormalize(right);CrossProduct(right,direction,up);
 VectorSubtract(c->center,light,d);scale=direction[3]/sqrtf(max(.0001f,1-direction[3]*direction[3]));
 x=DotProduct(d,right)*scale;y=DotProduct(d,up)*scale;z=DotProduct(d,direction);
 return z+r>=1 && fabsf(x)-z<=r*sqrtf(1+scale*scale) && fabsf(y)-z<=r*sqrtf(1+scale*scale);
}
void VK_ShadowRender(void) {
 int slot=Slot(),i,face,j,updates=0;
 VkCommandBuffer command=VK_CurrentCommandBuffer();shadow_uniform_t uniform={0};float view[16],projection[16],vp[16];
 VkRenderPassBeginInfo begin={0};VkClearValue clear={0};qbool begun=false;
 VkPipeline boundPipeline=VK_NULL_HANDLE;VkBuffer boundVertex=VK_NULL_HANDLE;VkDescriptorSet boundMaterial=VK_NULL_HANDLE;
 lastDraws=lastFaces=lastOverflow=lastCached=lastCulled=lastDeferred=0;
 if(!frames[slot].mapped||!command) return;
 if(!VK_ShadowWorkPending() || frames[slot].width==1) return;
 R_GetModelviewMatrix(view);R_GetProjectionMatrix(projection);R_MultiplyMatrix(view,projection,vp);
 if(!Inverse(vp,uniform.inverse)) Sys_Error("Vulkan: singular shadow receiver view");
 {VkViewport viewport;VkRect2D scissor;VK_SceneViewport(&viewport,&scissor);
 uniform.viewport[0]=viewport.width;uniform.viewport[1]=viewport.height;uniform.modes[2]=viewport.x;uniform.modes[3]=viewport.y;}
 VectorCopy(r_refdef.vieworg,uniform.camera);
 uniform.modes[0]=CV_Value(CV_maplights);uniform.modes[1]=CV_Value(CV_mapambient);
 uniform.viewport[2]=1.0f/frames[slot].width;uniform.viewport[3]=1.0f/frames[slot].height;
 uniform.settings[0]=(float)selectedCount;uniform.settings[1]=(float)(int)CV_Value(CV_shadowquality);uniform.settings[2]=CV_Value(CV_shadowbias);uniform.settings[3]=CV_Value(CV_shadowsoft);
 Pipeline(false);Pipeline(true);
 clear.depthStencil.depth=1;begin.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;begin.renderPass=pass;begin.framebuffer=frames[slot].framebuffer;
 begin.renderArea.extent.width=frames[slot].width;begin.renderArea.extent.height=frames[slot].height;begin.clearValueCount=1;begin.pClearValues=&clear;
 for(int order=0;order<selectedCount;++order) {
  i=(order+updateCursor[slot])%selectedCount;
  dlight_t *light=&lightData[selected[i]].light;qbool complete=GatherCasters(selected[i]);
  VectorCopy(light->origin,uniform.positions[i]);uniform.positions[i][3]=light->radius;
  for(j=0;j<3;++j) uniform.colors[i][j]=light->type==lt_custom?light->color[j]/255.0f:dlightcolor[bound(0,light->type,NUM_DLIGHTTYPES-1)][j]/128.0f;
  if(selected[i]>=MAX_DLIGHTS) {
   float style=d_lightstylevalue[lightData[selected[i]].style]/256.0f;
   for(j=0;j<3;++j)uniform.colors[i][j]*=style;
  }
  VectorCopy(lightData[selected[i]].direction,uniform.directions[i]);uniform.directions[i][3]=lightData[selected[i]].cone;
  /* Negative cone identifies an omnidirectional map light. */
  if(selected[i]>=MAX_DLIGHTS && !uniform.directions[i][3])uniform.directions[i][3]=-1;
  uniform.colors[i][3]=complete?CV_Value(CV_shadowstrength):0;
  if(!complete){frames[slot].hashes[i]=0;++lastOverflow;continue;} // Complete unshadowed light, never a partial occluder map.
  {
   unsigned long long hash=Hash(uniform.positions[i],16,1469598103934665603ULL);
   hash=Hash(&uniform.settings[1],4,hash);hash=Hash(uniform.directions[i],16,hash);hash=Hash(&cl.worldmodel,sizeof(cl.worldmodel),hash);
   for(j=0;j<casterCount;++j){shadow_caster_t c=casters[j];int a;for(a=0;a<16;++a)c.world[a]=roundf(c.world[a]*4096)/4096;hash=Hash(&c,sizeof(c),hash);}
   if(CV_Value(CV_shadowcache) && frames[slot].hashes[i]==hash){++lastCached;frames[slot].ages[i]=0;continue;}
   /* Dirty maps never use stale depth. Deferred lights stay fully illuminated. */
   if(updates>=(int)CV_Value(CV_shadowupdates)){uniform.colors[i][3]=0;++lastDeferred;continue;}
   ++updates;frames[slot].hashes[i]=hash;frames[slot].ages[i]=0;
  }
  if(!begun){vkCmdBeginRenderPass(command,&begin,VK_SUBPASS_CONTENTS_INLINE);begun=true;}
  for(face=0;face<(uniform.directions[i][3]>0?1:6);++face) {
   int tile=i*6+face;VkViewport viewport={(float)(tile%6*(int)uniform.settings[1]),(float)(tile/6*(int)uniform.settings[1]),uniform.settings[1],uniform.settings[1],0,1};
   VkRect2D scissor={{tile%6*(int)uniform.settings[1],tile/6*(int)uniform.settings[1]},{(uint32_t)uniform.settings[1],(uint32_t)uniform.settings[1]}};
   VkClearAttachment attachment={VK_IMAGE_ASPECT_DEPTH_BIT,0,{{0}}};VkClearRect rect={scissor,0,1};attachment.clearValue.depthStencil.depth=1;
   vkCmdClearAttachments(command,1,&attachment,1,&rect);
   vkCmdSetViewport(command,0,1,&viewport);vkCmdSetScissor(command,0,1,&scissor);++lastFaces;
   for(j=0;j<casterCount;++j) {
    shadow_caster_t *c=&casters[j];shadow_push_t push={0};VkDeviceSize offset=0;
    VkBuffer vertex=VK_BufferHandle(c->alias?r_buffer_aliasmodel_vertex_data:r_buffer_brushmodel_vertex_data);
    VkDescriptorSet material;
    if(uniform.directions[i][3]>0 ? !SpotRelevant(c,light->origin,uniform.directions[i]) : !FaceRelevant(c,light->origin,face)){++lastCulled;continue;}
    material=VK_TextureDescriptorSet(c->texture);
    if(!vertex||!material) continue;
    memcpy(push.world,c->world,64);memcpy(push.light,uniform.positions[i],16);memcpy(push.direction,uniform.directions[i],16);push.params[0]=(float)face;push.params[3]=c->lerp;
    if(boundPipeline!=pipelines[c->alias]){vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines[c->alias]);boundPipeline=pipelines[c->alias];}
    if(boundVertex!=vertex){vkCmdBindVertexBuffers(command,0,1,&vertex,&offset);boundVertex=vertex;}
    if(boundMaterial!=material){vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,&material,0,NULL);boundMaterial=material;}
    vkCmdPushConstants(command,pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(push),&push);
    vkCmdDraw(command,c->count,1,c->first,0);++lastDraws;
   }
  }
 }
 ++updateCursor[slot];if(begun)vkCmdEndRenderPass(command);frames[slot].initialized=true;
 memcpy(frames[slot].mapped,&uniform,sizeof(uniform));
}
void VK_ShadowStatus(void) {
 Con_Printf("SHADOW_STATE enabled=%d lights=%d ids=%d,%d faces=%u draws=%u overflow=%u size=%g alias=%d cached=%u culled=%u deferred=%u view=%d maplights=%d views=%d,%d,%d,%d\n",VK_ShadowEnabled(),selectedCount,selected[0],selected[1],lastFaces,lastDraws,lastOverflow,CV_Value(CV_shadowquality),aliasCount,lastCached,lastCulled,lastDeferred,viewIndex,mapLightCount,viewLights[0],viewLights[1],viewLights[2],viewLights[3]);

}
void VK_ShadowShutdown(void) {
 int i;
 VK_ShadowPipelineShutdown();
 for(i=0;i<VK_MAX_FRAMES_IN_FLIGHT*SHADOW_VIEWS;++i) DestroyFrame(i);
 Q_free(worldCasters);worldCasters=NULL;worldCasterCount=0;casterWorld=NULL;lightWorld=NULL;
 for(i=0;i<2;++i) if(pipelines[i]) vkDestroyPipeline(vk_options.logicalDevice,pipelines[i],NULL);
 if(pipelineLayout) vkDestroyPipelineLayout(vk_options.logicalDevice,pipelineLayout,NULL);
 if(pass) vkDestroyRenderPass(vk_options.logicalDevice,pass,NULL);
 if(sampler) vkDestroySampler(vk_options.logicalDevice,sampler,NULL);
 memset(frames,0,sizeof(frames));memset(pipelines,0,sizeof(pipelines));pipelineLayout=VK_NULL_HANDLE;pass=VK_NULL_HANDLE;sampler=VK_NULL_HANDLE;
 selectedCount=0;memset(selected,255,sizeof(selected));previousWorld=NULL;
}
#endif





