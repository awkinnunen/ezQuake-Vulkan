#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_hdr.glsl"
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 0) uniform sampler2D worldTexture[2];
layout(set = 1, binding = 0) uniform sampler2D lightmapTexture[2];
layout(set = 2, binding = 0) uniform sampler2D detailTexture[2];
layout(set = 3, binding = 0) uniform sampler2D causticsTexture[2];

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec2 inLightmapCoord;
layout(location = 2) in vec2 inDetailCoord;
layout(location = 3) flat in uint inFlags;

#include "vk_world_push.glsl"
#define CV_SET 4
#define CV_WORLD_FRAGMENT
#include "vk_competitive.glsl"
#include "vk_shadows.glsl"

layout(location = 0) out vec4 fragColour;

// EZQ_SURFACE_WORLD / EZQ_SURFACE_IS_FLOOR / EZQ_SURFACE_UNDERWATER from
// src/glsl/constants.glsl -- duplicated here since Vulkan shaders are
// compiled standalone (no #include).
#define EZQ_SURFACE_WORLD      64u
#define EZQ_SURFACE_IS_FLOOR   8u
#define EZQ_SURFACE_UNDERWATER 16u

// Port of GLC/GLM's applyColorTinting() (see draw_world.fragment.glsl) for
// r_drawflat_mode 1 (tinted) / 2 (bright) -- unlike mode 0, the real texture
// stays visible, just recolored.
vec3 applyDrawflatTint(vec3 colour)
{
	if ((inFlags & EZQ_SURFACE_WORLD) == 0u) {
		return colour;
	}

	bool isFloor = (inFlags & EZQ_SURFACE_IS_FLOOR) != 0u;
	float mixFloor = (isFloor && worldFlag(VK_WORLD_TINT_FLOORS)) ? 1.0 : 0.0;
	float mixWall = (!isFloor && worldFlag(VK_WORLD_TINT_WALLS)) ? 1.0 : 0.0;

	if (worldFlag(VK_WORLD_BRIGHT)) {
		// Bright: luminance-preserving recolor (kudos to Darel Rex Finley).
		float brightness = sqrt(colour.r * colour.r * 0.241 + colour.g * colour.g * 0.691 + colour.b * colour.b * 0.068);
		colour = mix(colour, unpackUnorm4x8(pushConstants.wallColor).rgb * brightness, mixWall);
		colour = mix(colour, unpackUnorm4x8(pushConstants.floorColor).rgb * brightness, mixFloor);
	}
	else if (worldFlag(VK_WORLD_TINTED)) {
		// Tinted: multiply.
		colour = mix(colour, colour * unpackUnorm4x8(pushConstants.floorColor).rgb, mixFloor);
		colour = mix(colour, colour * unpackUnorm4x8(pushConstants.wallColor).rgb, mixWall);
	}

	return colour;
}

void shadeScene()
{
	vec2 texCoord = inTexCoord;
	if (pushConstants.surfaceType > 0.5 && pushConstants.surfaceType < 5.5) {
		if (pushConstants.fastTurb > 0.5) {
			fragColour = vec4(hdrMaterial(pushConstants.color.rgb), 1.0);
			return;
		}

		texCoord.s += sin((inTexCoord.t + pushConstants.time) * 1.5) * 0.125;
		texCoord.t += sin((inTexCoord.s + pushConstants.time) * 1.5) * 0.125;
	}
	else if (worldFlag(VK_WORLD_TEXTURELESS)) {
		// Keep the lightmap/depth/outline pipeline exactly as-is, just
		// sample a single fixed texel from the world texture instead of
		// the surface's real UVs (same trick Modern OpenGL's
		// DRAW_TEXTURELESS uses for world geometry).
		texCoord = vec2(0.0);
	}

	vec4 texColour = texture(worldTexture[0], texCoord);
	if (pushConstants.surfaceType < 0.5) texColour.rgb = cvMaterial(worldTexture[0], texCoord, texColour.rgb, (inFlags & 8u) != 0u);
	vec4 lightColour = texture(lightmapTexture[0], inLightmapCoord);
	lightColour.rgb = cvLighting(shadowLighting(lightColour.rgb), false);

	if (texColour.a < 0.5) {
		discard;
	}

	fragColour = vec4(hdrMaterial(applyDrawflatTint(texColour.rgb)) * lightColour.rgb, 1.0);
	if (worldFlag(VK_WORLD_DETAIL)) {
		vec4 detail = texture(detailTexture[0], inDetailCoord);
		fragColour = vec4(detail.rgb * fragColour.rgb * 2.0, fragColour.a);
	}
	// Port of GLC/GLM's gl_caustics: an animated multiplicative overlay,
	// applied only to fragments flagged underwater at surface-build time
	// (see draw_world.fragment.glsl for the reference UV animation/blend).
	if (worldFlag(VK_WORLD_CAUSTICS) && (inFlags & EZQ_SURFACE_UNDERWATER) != 0u) {
		vec2 causticCoord = vec2(
			(inTexCoord.s + sin(0.465 * (pushConstants.time + inTexCoord.t))) * -0.1234375,
			(inTexCoord.t + sin(0.465 * (pushConstants.time + inTexCoord.s))) * -0.1234375);
		vec3 caustic = texture(causticsTexture[0], causticCoord).rgb;
		fragColour = vec4(caustic * fragColour.rgb * 2.0, fragColour.a);
	}
}

// HDR-001: emission follows the same coverage as color, independently of albedo.
layout(location=1) out vec4 fragEmission;
void main() {
 shadeScene();
 fragEmission=vec4(vec3(0), fragColour.a);
}
