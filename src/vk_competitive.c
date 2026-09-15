/* Competitive Visuals uniforms. OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifdef RENDERER_OPTION_VULKAN
#include <vulkan/vulkan.h>
#include "quakedef.h"
#include "vk_local.h"
#include "competitive_visuals.h"
#include "vk_shadows.h"

#define CV_DRAWS 8192
static VkDescriptorSetLayout cvLayout;
static VkDescriptorPool cvPool;
static VkDeviceSize cvStride;
static struct { VkBuffer buffer; VkDeviceMemory memory; byte *mapped; VkDescriptorSet set[4]; uint32_t used; } frames[VK_MAX_FRAMES_IN_FLIGHT];
typedef char cv_params_size_check[(sizeof(cv_params_t)==224)?1:-1];

VkDescriptorSetLayout VK_CVLayout(void) {
 VkDescriptorSetLayoutBinding binding={0}; VkDescriptorSetLayoutCreateInfo info={0};
 if(cvLayout) return cvLayout;
 binding.binding=0; binding.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
 binding.descriptorCount=1; binding.stageFlags=VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT;
 info.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO; info.bindingCount=1;info.pBindings=&binding;
 VkDescriptorSetLayoutBinding bindings[3]={binding,{0},{0}};
 bindings[1].binding=1;bindings[1].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;bindings[1].descriptorCount=1;bindings[1].stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;
 bindings[2].binding=2;bindings[2].descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;bindings[2].descriptorCount=1;bindings[2].stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;
 info.bindingCount=3;info.pBindings=bindings;
 if(vkCreateDescriptorSetLayout(vk_options.logicalDevice,&info,NULL,&cvLayout)!=VK_SUCCESS) Sys_Error("Vulkan: Competitive Visuals layout failed");
 return cvLayout;
}
void VK_CVBeginFrame(void) {
 uint32_t i=vk_options.frame.currentFrame;
 cv_params_t params;
 VK_WorldBeginFrame();
 VK_ShadowFrameResources();
 if(!cvPool) {
  VkDescriptorPoolSize sizes[3]={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,VK_MAX_FRAMES_IN_FLIGHT*4},{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_MAX_FRAMES_IN_FLIGHT*4},{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_MAX_FRAMES_IN_FLIGHT*4}};
  VkDescriptorPoolCreateInfo info={0}; VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(vk_options.physicalDevice,&props);
  if(props.limits.maxBoundDescriptorSets<5) Sys_Error("Vulkan: Competitive Visuals pipelines require 5 descriptor sets");
  cvStride=(sizeof(cv_params_t)+props.limits.minUniformBufferOffsetAlignment-1)&~(props.limits.minUniformBufferOffsetAlignment-1);
  info.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;info.maxSets=VK_MAX_FRAMES_IN_FLIGHT*4;info.poolSizeCount=3;info.pPoolSizes=sizes;
  if(vkCreateDescriptorPool(vk_options.logicalDevice,&info,NULL,&cvPool)!=VK_SUCCESS) Sys_Error("Vulkan: Competitive Visuals pool failed");
 }
 if(!frames[i].buffer) {

  if(!VK_CreateBufferResource(cvStride*CV_DRAWS,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&frames[i].buffer,&frames[i].memory)) Sys_Error("Vulkan: Competitive Visuals buffer failed");
  if(vkMapMemory(vk_options.logicalDevice,frames[i].memory,0,VK_WHOLE_SIZE,0,(void**)&frames[i].mapped)!=VK_SUCCESS) Sys_Error("Vulkan: Competitive Visuals map failed");
 }
 for(int v=0;v<4;++v) {
  VkDescriptorSetAllocateInfo alloc={0};VkDescriptorBufferInfo buf={0};VkWriteDescriptorSet write={0};VkDescriptorSetLayout layout=VK_CVLayout();
  alloc.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;alloc.descriptorPool=cvPool;alloc.descriptorSetCount=1;alloc.pSetLayouts=&layout;
  if(!frames[i].set[v] && vkAllocateDescriptorSets(vk_options.logicalDevice,&alloc,&frames[i].set[v])!=VK_SUCCESS) Sys_Error("Vulkan: Competitive Visuals descriptor failed");
  buf.buffer=frames[i].buffer;buf.range=sizeof(cv_params_t);
  write.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;write.dstSet=frames[i].set[v];write.dstBinding=0;
  write.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;write.descriptorCount=1;write.pBufferInfo=&buf;
  vkUpdateDescriptorSets(vk_options.logicalDevice,1,&write,0,NULL);
  VkDescriptorImageInfo shadowImage={0};VkDescriptorBufferInfo shadowBuffer={0};VkWriteDescriptorSet shadowWrites[2]={{0}};
  VK_ShadowDescriptors(v,&shadowImage,&shadowBuffer);
  shadowWrites[0].sType=shadowWrites[1].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  shadowWrites[0].dstSet=shadowWrites[1].dstSet=frames[i].set[v];
  shadowWrites[0].dstBinding=1;shadowWrites[0].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;shadowWrites[0].descriptorCount=1;shadowWrites[0].pImageInfo=&shadowImage;
  shadowWrites[1].dstBinding=2;shadowWrites[1].descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;shadowWrites[1].descriptorCount=1;shadowWrites[1].pBufferInfo=&shadowBuffer;
  vkUpdateDescriptorSets(vk_options.logicalDevice,2,shadowWrites,0,NULL);
 }
 /* Called only after this frame slot's fence has completed. Draw slices are
    immutable until that fence completes again, including unsubmitted records. */
 frames[i].used=1; CV_Params(&params);memcpy(frames[i].mapped,&params,sizeof(params));
}
void VK_CVBind(VkCommandBuffer command, VkPipelineLayout layout, uint32_t set, const void *params) {
 uint32_t i=vk_options.frame.currentFrame,offset=0;int v=VK_ShadowViewIndex();
 if(params) {
  if(frames[i].used>=CV_DRAWS) Sys_Error("Vulkan: Competitive Visuals draw capacity exceeded");
  offset=(uint32_t)(cvStride*frames[i].used++);
  memcpy(frames[i].mapped+offset,params,sizeof(cv_params_t));
 }
 vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_GRAPHICS,layout,set,1,&frames[i].set[v],1,&offset);
}
void VK_CVShutdown(void) {
 VK_ShadowShutdown();
 int i;
 for(i=0;i<VK_MAX_FRAMES_IN_FLIGHT;++i) {
  if(frames[i].mapped) vkUnmapMemory(vk_options.logicalDevice,frames[i].memory);
  if(frames[i].buffer) vkDestroyBuffer(vk_options.logicalDevice,frames[i].buffer,NULL);
  if(frames[i].memory) vkFreeMemory(vk_options.logicalDevice,frames[i].memory,NULL);
 }
 if(cvPool) vkDestroyDescriptorPool(vk_options.logicalDevice,cvPool,NULL);
 if(cvLayout) vkDestroyDescriptorSetLayout(vk_options.logicalDevice,cvLayout,NULL);
 memset(frames,0,sizeof(frames));cvPool=VK_NULL_HANDLE;cvLayout=VK_NULL_HANDLE;cvStride=0;
}
#endif
