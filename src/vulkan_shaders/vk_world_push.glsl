// Must match vk_world_push_t: 128 bytes. OpenAI Codex, 2026-09-13. GPL-2.0-or-later.
#include "vk_world_flags.h"
layout(push_constant) uniform PushConstants {
	mat4 mvp;
	vec4 color;
	vec4 cameraPosition;
	float time;
	float alpha;
	float surfaceType;
	float useSkyTexture;
	float fastTurb;
	uint floorColor;
	uint wallColor;
	uint flags;
} pushConstants;

bool worldFlag(uint flag)
{
	return (pushConstants.flags & flag) != 0u;
}
