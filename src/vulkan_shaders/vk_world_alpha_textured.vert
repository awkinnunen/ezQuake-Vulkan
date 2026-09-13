#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec2 inDetailCoord;
layout(location = 3) in uint inFlagsAttrib;

#include "vk_world_push.glsl"

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec2 outDetailCoord;
layout(location = 2) flat out uint outFlags;

void main()
{
	vec4 clip = pushConstants.mvp * vec4(inPosition, 1.0);

	clip.y = -clip.y;
	clip.z = clip.z * 0.5 + clip.w * 0.5;

	gl_Position = clip;
	outTexCoord = inTexCoord;
	outDetailCoord = inDetailCoord;
	outFlags = inFlagsAttrib;
}
