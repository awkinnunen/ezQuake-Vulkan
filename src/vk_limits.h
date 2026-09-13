/* Local implementation: OpenAI Codex, 2026-09-13. GPL-2.0-or-later.
 * Pure limit checks shared by the renderer and its boundary tests. */
#ifndef EZQUAKE_VK_LIMITS_H
#define EZQUAKE_VK_LIMITS_H
#include <vulkan/vulkan.h>

#define VK_WORLD_PUSH_BYTES 128u

static inline int VK_PushRangeFits(uint32_t limit, const VkPushConstantRange* range)
{
	return range->stageFlags && range->size && !(range->offset & 3u) &&
		!(range->size & 3u) && range->offset <= limit &&
		range->size <= limit - range->offset;
}

/* Both fixed arrays contain combined image samplers. Each descriptor counts
 * against both the sampler and sampled-image limits, and once as a resource.
 * This renderer owns one update-after-bind pool containing this one set. */
static inline const char* VK_BindlessLimitFailure(
	const VkPhysicalDeviceDescriptorIndexingProperties* limits, uint32_t count)
{
#define VK_CHECK_BINDLESS_LIMIT(field) if (count > limits->field) return #field
	VK_CHECK_BINDLESS_LIMIT(maxPerStageDescriptorUpdateAfterBindSamplers);
	VK_CHECK_BINDLESS_LIMIT(maxPerStageDescriptorUpdateAfterBindSampledImages);
	VK_CHECK_BINDLESS_LIMIT(maxPerStageUpdateAfterBindResources);
	VK_CHECK_BINDLESS_LIMIT(maxDescriptorSetUpdateAfterBindSamplers);
	VK_CHECK_BINDLESS_LIMIT(maxDescriptorSetUpdateAfterBindSampledImages);
	VK_CHECK_BINDLESS_LIMIT(maxUpdateAfterBindDescriptorsInAllPools);
#undef VK_CHECK_BINDLESS_LIMIT
	return NULL;
}
#endif
