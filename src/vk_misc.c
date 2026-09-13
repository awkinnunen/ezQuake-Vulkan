
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

#ifdef RENDERER_OPTION_VULKAN

#include "quakedef.h"
#include "vk_local.h"
#include "r_texture.h"

void VK_PrintGfxInfo(void)
{
	const VkPhysicalDeviceProperties* p = &vk_options.physicalDeviceProperties;
	const VkPhysicalDeviceDescriptorIndexingProperties* d = &vk_options.descriptorIndexingProperties;
	Con_Printf("vulkan: device %s, API %u.%u.%u\n", p->deviceName,
		VK_VERSION_MAJOR(p->apiVersion), VK_VERSION_MINOR(p->apiVersion), VK_VERSION_PATCH(p->apiVersion));
	Con_Printf("vulkan: push constants %u bytes (world requires %u), descriptor sets %u\n",
		p->limits.maxPushConstantsSize, VK_WORLD_PUSH_BYTES, p->limits.maxBoundDescriptorSets);
	Con_Printf("vulkan: descriptor indexing %s, bindless table %u combined image samplers\n",
		vk_options.supportsDescriptorIndexing ? "enabled" : "unavailable", 2u * MAX_GLTEXTURES);
	Con_Printf("vulkan: update-after-bind limits: stage samplers/images/resources %u/%u/%u, set samplers/images %u/%u, pools %u\n",
		d->maxPerStageDescriptorUpdateAfterBindSamplers, d->maxPerStageDescriptorUpdateAfterBindSampledImages,
		d->maxPerStageUpdateAfterBindResources, d->maxDescriptorSetUpdateAfterBindSamplers,
		d->maxDescriptorSetUpdateAfterBindSampledImages, d->maxUpdateAfterBindDescriptorsInAllPools);
}

#endif // #ifdef RENDERER_OPTION_VULKAN
