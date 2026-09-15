/*
Copyright (C) 2018 ezQuake team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// vk_vao.c
// - Vulkan VAO-equivalent functions

#ifdef RENDERER_OPTION_VULKAN

#include <vulkan/vulkan.h>
#include "quakedef.h"
#include "r_state.h"

#include "vk_local.h"
#include "competitive_visuals.h"
#include "vk_shadows.h"

static VkFormat sceneFormat;
VkFormat VK_SceneFormat(void) { return sceneFormat; }
qbool VK_HDRActive(void) { return sceneFormat == VK_FORMAT_R16G16B16A16_SFLOAT; }
void VK_ConfigureSceneFormat(void)
{
	VkFormatProperties format;
	VkImageFormatProperties image;
	VkFormatFeatureFlags needed = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
		VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
	int requested;
	// Video starts before M_Init. Adopt saved/temp cvars before selecting the
	// first scene target, not only after a later user-triggered restart.
	CV_InitSettings();
	requested = (int)CV_Value(CV_hdr);
	sceneFormat = vk_options.physicalDeviceSurfaceFormat.format;
	vkGetPhysicalDeviceFormatProperties(vk_options.physicalDevice, VK_FORMAT_R16G16B16A16_SFLOAT, &format);
	if (requested && (format.optimalTilingFeatures & needed) == needed &&
		vkGetPhysicalDeviceImageFormatProperties(vk_options.physicalDevice, VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		0, &image) == VK_SUCCESS && (image.sampleCounts & vk_options.msaaSamples)) {
		sceneFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	}
	CV_HDRVideoApplied(requested, VK_HDRActive());
	Con_Printf("vulkan: scene format %s (HDR requested %d, active %d)\n",
		VK_HDRActive() ? "RGBA16F linear" : "SDR compatibility", requested, VK_HDRActive());
	if (requested && !VK_HDRActive()) Con_Printf("vulkan: HDR format/sample combination unsupported; using SDR. Try lower MSAA.\n");
}

// Specialization is immutable per pipeline; HUD shaders have no constant 31.
static VkResult ScenePipeline(const VkGraphicsPipelineCreateInfo *info, VkPipeline *pipeline, qbool shadows)
{
	VkGraphicsPipelineCreateInfo copy = *info;
	VkPipelineShaderStageCreateInfo stages[2];
	VkBool32 flags[3] = {VK_HDRActive(), VK_FALSE, shadows};
	VkSpecializationMapEntry entries[3] = {{31, 0, sizeof(VkBool32)}, {32, sizeof(VkBool32), sizeof(VkBool32)}, {33, 2*sizeof(VkBool32), sizeof(VkBool32)}};
	VkSpecializationInfo spec = {3, entries, sizeof(flags), flags};
	VkPipelineColorBlendStateCreateInfo blend;
	VkPipelineColorBlendAttachmentState attachments[2];
	uint32_t i;
	if (info->pColorBlendState && info->pColorBlendState->attachmentCount) {
		const VkPipelineColorBlendAttachmentState *b=info->pColorBlendState->pAttachments;
		flags[1]=b->blendEnable && b->dstColorBlendFactor==VK_BLEND_FACTOR_ONE;
	}
	assert(info->stageCount <= 2);
	for (i=0; i<info->stageCount; ++i) {
		stages[i] = info->pStages[i];
		assert(!stages[i].pSpecializationInfo);
		if (stages[i].stage == VK_SHADER_STAGE_FRAGMENT_BIT) stages[i].pSpecializationInfo = &spec;
	}
	copy.pStages = stages;
	if (VK_HDRActive() && info->renderPass == VK_MainRenderPass()) {
		blend = *info->pColorBlendState;
		attachments[0] = attachments[1] = blend.pAttachments[0];
		blend.attachmentCount = 2;
		blend.pAttachments = attachments;
		copy.pColorBlendState = &blend;
	}
	return vkCreateGraphicsPipelines(vk_options.logicalDevice, vk_options.pipelineCache, 1, &copy, NULL, pipeline);
}

// SHADOW-002: specialization removes the shadow loop and its register cost when off.
static struct {VkPipeline on,off;} shadowVariants[64];
static int shadowVariantCount;
VkResult VK_CreateScenePipeline(const VkGraphicsPipelineCreateInfo *info,VkPipeline *pipeline) {return ScenePipeline(info,pipeline,true);}
VkResult VK_CreateShadowScenePipeline(const VkGraphicsPipelineCreateInfo *info,VkPipeline *pipeline) {
 VkResult result;VkPipeline off;
 if(shadowVariantCount==64) return VK_ERROR_TOO_MANY_OBJECTS;
 result=ScenePipeline(info,&off,false);if(result!=VK_SUCCESS)return result;
 result=ScenePipeline(info,pipeline,true);
 if(result!=VK_SUCCESS){vkDestroyPipeline(vk_options.logicalDevice,off,NULL);return result;}
 shadowVariants[shadowVariantCount].on=*pipeline;shadowVariants[shadowVariantCount++].off=off;return result;
}
VkPipeline VK_ShadowScenePipeline(VkPipeline pipeline) {
 int i;if(VK_ShadowWorkPending())return pipeline;
 for(i=0;i<shadowVariantCount;++i)if(shadowVariants[i].on==pipeline)return shadowVariants[i].off;
 return pipeline;
}
void VK_ShadowPipelineShutdown(void) {
 int i;for(i=0;i<shadowVariantCount;++i)vkDestroyPipeline(vk_options.logicalDevice,shadowVariants[i].off,NULL);
 shadowVariantCount=0;
}

typedef enum {
	vk_renderpass_main,
	// Same attachments/subpass/dependency as vk_renderpass_main, except the
	// color attachment uses LOAD instead of CLEAR -- backs "Clear Video
	// Buffer" off (gl_clear 0). Render pass compatibility (Vulkan spec 7.2)
	// only depends on attachment format/sample count, not loadOp/layouts, so
	// pipelines and framebuffers built against vk_renderpass_main are equally
	// valid to use with this one; only vkCmdBeginRenderPass needs to pick
	// between them, done once per frame in VK_BeginFrame().
	vk_renderpass_main_noclear,
	// PERF-OPT-002, OpenAI Codex: single-view offscreen pass is terminal.
	// Its resolved color is sampled; multisample color is never LOADed again.
	vk_renderpass_main_terminal,
	// Second pass used only when VK_PostProcessActive() is true: a single
	// fullscreen-quad subpass that reads the offscreen color target the main
	// pass just wrote (see VK_CreatePostProcessResources) via sampler, applies
	// real gamma/contrast/FXAA, and writes the swapchain image directly --
	// see VK_PostProcessComposite in vk_draw.c.
	vk_renderpass_postprocess,
	vk_renderpass_hud,
	// World-outline normals prepass (gl_outline & 2): a small, always
	// single-sample render pass with its own color (normal+linear-depth,
	// RGBA16F) and depth attachments, entirely separate from the main
	// render pass's MSAA/post-process attachment matrix -- see
	// VK_CreateWorldNormalsResources in vk_swapchain.c for why. Its color
	// attachment is later sampled (not presented) by the outline composite
	// pipeline drawn inline in the main render pass.
	vk_renderpass_worldnormals,

	vk_renderpass_count
} vk_renderpass_id;

static VkRenderPass renderPasses[vk_renderpass_count];

static qbool VK_RenderPassCreateVariant(vk_renderpass_id id, qbool clearColor)
{
	qbool msaa = vk_options.msaaSamples > VK_SAMPLE_COUNT_1_BIT;
	VkAttachmentDescription attachments[5];
	VkAttachmentReference colors[2], resolves[2];
	VkAttachmentReference colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef;
	VkAttachmentReference resolveAttachmentRef;
	VkSubpassDescription subpass;
	VkSubpassDependency dependency;
	VkRenderPassCreateInfo renderPassInfo;

	// Attachment 0: the color attachment every pipeline actually draws into.
	// Without MSAA this is the scene image (swapchain in the direct path,
	// otherwise offscreen). With MSAA it resolves into attachment 2.
	// Only a terminal single-view offscreen pass can discard MSAA colour;
	// resumed/direct LOAD paths must retain the multisample contents.
	VK_InitialiseStructure(attachments[0]);
	attachments[0].format = VK_SceneFormat();
	attachments[0].samples = vk_options.msaaSamples ? vk_options.msaaSamples : VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = (msaa && id == vk_renderpass_main_terminal) ?
		VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	if (msaa || VK_HDRActive()) {
		// MSAA/HDR images are render targets, never directly presented. A
		// resumed pass preserves their contents in COLOR_ATTACHMENT_OPTIMAL.
		attachments[0].initialLayout = clearColor ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else {
		// LOAD requires the attachment to already be in the layout it's
		// loaded from -- swapchain images sit in PRESENT_SRC_KHR between
		// frames (that's this same render pass's finalLayout below), so
		// that's what the no-clear variant declares as its initialLayout to
		// preserve content instead of triggering an undefined-content
		// layout transition.
		attachments[0].initialLayout = clearColor ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	VK_InitialiseStructure(attachments[1]);
	attachments[1].format = VK_DepthFormat();
	attachments[1].samples = attachments[0].samples;
	// Depth always clears regardless of gl_clear -- matches GL_Clear() in
	// gl_misc.c, which only gates GL_COLOR_BUFFER_BIT on clear_color and
	// always ORs in GL_DEPTH_BUFFER_BIT.
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Every scene pass CLEARs depth; no depth sampling or depth resolve exists.
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Attachment 2 (MSAA only): the actual swapchain image, written only by
	// the resolve at the end of the subpass -- same PRESENT_SRC_KHR
	// load/store semantics the non-MSAA attachment 0 has above, just moved
	// here since the swapchain image itself is never drawn into directly
	// when multisampling.
	if (msaa) {
		VK_InitialiseStructure(attachments[2]);
		attachments[2].format = VK_SceneFormat();
		attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout = clearColor ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		if (VK_HDRActive()) {
			attachments[2].initialLayout = clearColor ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	// attachment reference
	VK_InitialiseStructure(colorAttachmentRef);
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(depthAttachmentRef);
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(resolveAttachmentRef);
	resolveAttachmentRef.attachment = 2;
	resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Sub-passes
	VK_InitialiseStructure(subpass);
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = msaa ? &resolveAttachmentRef : NULL;
	if (VK_HDRActive()) {
		// A separate linear emission target excludes ordinary bright albedo
		// from emissive bloom. Same blend/coverage rules as scene color.
		int emission = msaa ? 3 : 2;
		attachments[emission] = attachments[0];
		colors[0] = colorAttachmentRef;
		colors[1].attachment = emission;
		colors[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		subpass.colorAttachmentCount = 2;
		subpass.pColorAttachments = colors;
		if (msaa) {
			attachments[4] = attachments[2];
			resolves[0] = resolveAttachmentRef;
			resolves[1].attachment = 4;
			resolves[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			subpass.pResolveAttachments = resolves;
		}
	}

	// The depth image (unlike the swapchain color image) is a single resource
	// shared by every frame in flight, not duplicated per-frame. EARLY_FRAGMENT_TESTS
	// alone only covers the depth *test*; the actual depth *write* for a passing
	// fragment retires in LATE_FRAGMENT_TESTS. Without it here, this renderpass's
	// CLEAR/write of the depth attachment can start before the previous frame's
	// depth writes have actually landed once the GPU is fed fast enough (high,
	// uncapped fps) to have two frames' worth of fragment work overlapping --
	// observed as walls flickering/briefly showing through, worse at high fps,
	// almost gone when fps is capped lower.
	VK_InitialiseStructure(dependency);
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Render pass
	VK_InitialiseStructure(renderPassInfo);
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = msaa ? 3 : 2;
	if (VK_HDRActive()) renderPassInfo.attachmentCount = msaa ? 5 : 3;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	{
		VkResult result = vkCreateRenderPass(vk_options.logicalDevice, &renderPassInfo, NULL, &renderPasses[id]);
		if (result != VK_SUCCESS) {
			Com_Printf("vulkan: vkCreateRenderPass() failed: %d\n", result);
			return false;
		}
	}

	return true;
}

qbool VK_RenderPassCreate(void)
{
	return VK_RenderPassCreateVariant(vk_renderpass_main, true) &&
		VK_RenderPassCreateVariant(vk_renderpass_main_noclear, false) &&
		VK_RenderPassCreateVariant(vk_renderpass_main_terminal, true);
}

// Single subpass, single color attachment (the swapchain image), no depth --
// pure composition. Source attachment (the offscreen target) is bound as a
// sampled descriptor by the pipeline in vk_draw.c, not as a render-pass
// attachment, so no input-attachment subpass dependency is needed here beyond
// the usual external one.
static qbool VK_PostProcessRenderPassCreate(qbool hud)
{
	VkAttachmentDescription colorAttachment;
	VkAttachmentReference colorAttachmentRef;
	VkSubpassDescription subpass;
	VkSubpassDependency dependency;
	VkRenderPassCreateInfo renderPassInfo;

	VK_InitialiseStructure(colorAttachment);
	colorAttachment.format = vk_options.physicalDeviceSurfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = hud ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = hud ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VK_InitialiseStructure(colorAttachmentRef);
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(subpass);
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// The read hazard (sampling the offscreen target) is handled explicitly by
	// the VK_PostProcessTransitionForSampling barrier before this render pass
	// begins, not by this dependency -- this only needs to order against
	// whatever previously used the swapchain image (the presentation engine,
	// same as the main render pass's own external dependency pattern).
	VK_InitialiseStructure(dependency);
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

	VK_InitialiseStructure(renderPassInfo);
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(vk_options.logicalDevice, &renderPassInfo, NULL, &renderPasses[hud ? vk_renderpass_hud : vk_renderpass_postprocess]) == VK_SUCCESS;
}

// See VK_WorldNormalsFormat above for the color format. Always
// single-sample regardless of vid_framebuffer_multisample -- the whole point
// of this being a separate render pass instead of a second attachment on the
// main one is to sidestep having to make normals+depth track the main pass's
// MSAA on/off and post-process on/off state (4 combinations already; adding
// this attachment there would multiply that to 8). CLEAR is mandatory on the
// color attachment (not LOAD/DONT_CARE): the composite pass distinguishes
// "surface drawn here" from "nothing drawn here" by alpha, and undefined
// contents on skipped pixels would make that unreliable.
static qbool VK_WorldNormalsRenderPassCreate(void)
{
	VkAttachmentDescription attachments[2];
	VkAttachmentReference colorAttachmentRef;
	VkAttachmentReference depthAttachmentRef;
	VkSubpassDescription subpass;
	VkSubpassDependency dependency;
	VkRenderPassCreateInfo renderPassInfo;

	VK_InitialiseStructure(attachments[0]);
	attachments[0].format = VK_WorldNormalsFormat();
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VK_InitialiseStructure(attachments[1]);
	attachments[1].format = VK_DepthFormat();
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(colorAttachmentRef);
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(depthAttachmentRef);
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VK_InitialiseStructure(subpass);
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Ordered before the main render pass's own external dependency in
	// VK_BeginFrame -- this only needs to make sure nothing reads this
	// attachment as a sampler (the composite pipeline's read, later in the
	// same command buffer within the main render pass) before this pass's
	// color write actually lands.
	VK_InitialiseStructure(dependency);
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VK_InitialiseStructure(renderPassInfo);
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return vkCreateRenderPass(vk_options.logicalDevice, &renderPassInfo, NULL, &renderPasses[vk_renderpass_worldnormals]) == VK_SUCCESS;
}

VkRenderPass VK_MainRenderPass(void)
{
	return renderPasses[vk_renderpass_main];
}

VkRenderPass VK_FrameRenderPass(qbool clear_color)
{
	// Deferred normals run before the single-view scene. Multiview can resume
	// a pass, and direct gl_clear 0 frames can LOAD prior color: retain STORE.
	if (clear_color && vk_options.swapChain.postProcessActive && !cl_multiview.integer)
		return renderPasses[vk_renderpass_main_terminal];
	return renderPasses[clear_color ? vk_renderpass_main : vk_renderpass_main_noclear];
}

VkRenderPass VK_PostProcessRenderPass(void)
{
	if (renderPasses[vk_renderpass_postprocess] == VK_NULL_HANDLE) {
		VK_PostProcessRenderPassCreate(false);
	}
	return renderPasses[vk_renderpass_postprocess];
}

VkRenderPass VK_HudRenderPass(void)
{
	if (!renderPasses[vk_renderpass_hud]) VK_PostProcessRenderPassCreate(true);
	return renderPasses[vk_renderpass_hud];
}

VkRenderPass VK_WorldNormalsRenderPass(void)
{
	if (renderPasses[vk_renderpass_worldnormals] == VK_NULL_HANDLE) {
		VK_WorldNormalsRenderPassCreate();
	}
	return renderPasses[vk_renderpass_worldnormals];
}

VkFormat VK_DepthFormat(void)
{
	return VK_FORMAT_D32_SFLOAT;
}

// RGBA16F: .rgb is the world-space face normal (from dFdx/dFdy of the
// interpolated world position in vk_world_normals.frag -- exact for Quake's
// planar BSP faces, not an approximation), .a is GLM's surfaceType-or-depth
// sentinel (see draw_world.fragment.glsl's normals pass and
// vk_world_normals.frag). Octahedral-encoded R16G16 would halve this, but
// GLM's own .a-as-depth-sentinel trick doesn't fit in 2 channels -- left as a
// follow-up if memory/bandwidth on mobile ever needs it (desktop-only path
// for now, see VK_MAX_FRAMES_IN_FLIGHT's comment on this branch).
VkFormat VK_WorldNormalsFormat(void)
{
	return VK_FORMAT_R16G16B16A16_SFLOAT;
}

void VK_RenderPassDelete(void)
{
	int i;

	for (i = 0; i < vk_renderpass_count; ++i) {
		if (renderPasses[i]) {
			vkDestroyRenderPass(vk_options.logicalDevice, renderPasses[i], NULL);
			renderPasses[i] = VK_NULL_HANDLE;
		}
	}
}

#endif // RENDERER_OPTION_VULKAN
