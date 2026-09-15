#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_hdr.glsl"

layout(set = 0, binding = 0) uniform sampler2D spriteTexture[2];

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec4 inColor;

layout(push_constant) uniform PushConstants {
	mat4 mvp;
	float alphaThreshold;
} pushConstants;

layout(location = 0) out vec4 fragColour;

void shadeScene()
{
	vec4 texColour = texture(spriteTexture[0], inTexCoord);

	fragColour = vec4(hdrMaterial(texColour.rgb) * hdrMaterial(inColor.rgb), texColour.a * inColor.a);
	if (pushConstants.alphaThreshold > 0.0 && fragColour.a <= pushConstants.alphaThreshold) {
		discard;
	}
}

// HDR-001: emission follows the same coverage as color, independently of albedo.
layout(location=1) out vec4 fragEmission;
void main() {
 shadeScene();
 fragEmission=vec4(hdrAdditive ? fragColour.rgb : vec3(0), fragColour.a);
}
