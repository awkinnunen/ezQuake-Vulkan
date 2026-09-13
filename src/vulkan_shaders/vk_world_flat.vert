#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inFlatColor;
layout(location = 2) in vec3 inLightmapCoord;

#include "vk_world_push.glsl"

layout(location = 0) out vec3 outFlatColor;
layout(location = 1) out vec3 outDirection;
layout(location = 2) out vec2 outLightmapCoord;

void main()
{
	vec4 clip = pushConstants.mvp * vec4(inPosition, 1.0);

	clip.y = -clip.y;
	clip.z = clip.z * 0.5 + clip.w * 0.5;

	gl_Position = clip;
	outFlatColor = inFlatColor;
	outDirection = inPosition - pushConstants.cameraPosition.xyz;
	outLightmapCoord = inLightmapCoord.xy;
}
