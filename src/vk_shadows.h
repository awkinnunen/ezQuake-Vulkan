/* SHADOW-001, OpenAI Codex, 2026-09-14. GPL-2.0-or-later. */
#ifndef EZ_VK_SHADOWS_H
#define EZ_VK_SHADOWS_H
qbool VK_ShadowManagedLight(int index);
qbool VK_ShadowCasterRelevant(entity_t *ent);
#ifdef VK_VERSION_1_0
void VK_WorldBeginFrame(void);
VkResult VK_CreateShadowScenePipeline(const VkGraphicsPipelineCreateInfo *info,VkPipeline *pipeline);
VkPipeline VK_ShadowScenePipeline(VkPipeline pipeline);
void VK_ShadowPipelineShutdown(void);
void VK_ShadowFrameResources(void);
void VK_ShadowSelect(void);
void VK_ShadowRender(void);
void VK_ShadowBeginCommands(void);
qbool VK_ShadowEnabled(void);
qbool VK_ShadowWorkPending(void);
void VK_ShadowShutdown(void);
void VK_SceneViewport(VkViewport *viewport,VkRect2D *scissor);
int VK_ShadowViewIndex(void);
void VK_ShadowDescriptors(int view,VkDescriptorImageInfo *image, VkDescriptorBufferInfo *buffer);
void VK_ShadowAddAlias(unsigned first, unsigned count, const float *modelview, float lerp, texture_ref texture, float radius);
void VK_ShadowStatus(void);
#endif
#endif
