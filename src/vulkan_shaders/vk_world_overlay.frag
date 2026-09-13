#version 450
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 0) uniform sampler2D overlayTexture[2];
layout(set = 1, binding = 0) uniform sampler2D detailTexture[2];

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec2 inDetailCoord;

#include "vk_world_push.glsl"

layout(location = 0) out vec4 fragColour;

void main()
{
	vec4 texColour = texture(overlayTexture[0], inTexCoord);

	if (worldFlag(VK_WORLD_DETAIL)) {
		vec4 detail = texture(detailTexture[0], inDetailCoord);
		texColour = vec4(detail.rgb * texColour.rgb * 2.0, texColour.a);
	}

	if (texColour.a <= 0.0 && max(max(texColour.r, texColour.g), texColour.b) <= 0.0) {
		discard;
	}

	fragColour = texColour;
}
