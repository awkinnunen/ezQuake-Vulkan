#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_hdr.glsl"
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec3 inFlatColor;
layout(location = 1) in vec3 inDirection;
layout(location = 2) in vec2 inLightmapCoord;

layout(set = 0, binding = 0) uniform sampler2D skyTexture;
layout(set = 0, binding = 1) uniform sampler2D skyCloudTexture;
layout(set = 0, binding = 2) uniform sampler2D skyboxFace0;
layout(set = 0, binding = 3) uniform sampler2D skyboxFace1;
layout(set = 0, binding = 4) uniform sampler2D skyboxFace2;
layout(set = 0, binding = 5) uniform sampler2D skyboxFace3;
layout(set = 0, binding = 6) uniform sampler2D skyboxFace4;
layout(set = 0, binding = 7) uniform sampler2D skyboxFace5;
layout(set = 1, binding = 0) uniform sampler2D lightmapTexture[2];

#include "vk_world_push.glsl"
#define CV_SET 4
#define CV_WORLD_FRAGMENT
#include "vk_competitive.glsl"
#include "vk_shadows.glsl"
#include "vk_skybox_uv.glsl"

layout(location = 0) out vec4 fragColour;

int skyboxAxis(vec3 dir)
{
	vec3 adir = abs(dir);

	if (adir.x > adir.y && adir.x > adir.z) {
		return dir.x < 0.0 ? 1 : 0;
	}
	if (adir.y > adir.z && adir.y > adir.x) {
		return dir.y < 0.0 ? 3 : 2;
	}
	return dir.z < 0.0 ? 5 : 4;
}

ivec2 skyboxFaceSize(int axis)
{
	if (axis == 0) return textureSize(skyboxFace0, 0);
	if (axis == 1) return textureSize(skyboxFace1, 0);
	if (axis == 2) return textureSize(skyboxFace2, 0);
	if (axis == 3) return textureSize(skyboxFace3, 0);
	if (axis == 4) return textureSize(skyboxFace4, 0);
	return textureSize(skyboxFace5, 0);
}

vec2 skyboxUv(int axis, vec3 dir)
{
	float s;
	float t;
	float dv;

	if (axis == 0) {
		dv = dir.x;
		s = -dir.y / dv;
		t = dir.z / dv;
	}
	else if (axis == 1) {
		dv = -dir.x;
		s = dir.y / dv;
		t = dir.z / dv;
	}
	else if (axis == 2) {
		dv = dir.y;
		s = dir.x / dv;
		t = dir.z / dv;
	}
	else if (axis == 3) {
		dv = -dir.y;
		s = -dir.x / dv;
		t = dir.z / dv;
	}
	else if (axis == 4) {
		dv = dir.z;
		s = -dir.y / dv;
		t = -dir.x / dv;
	}
	else {
		dv = -dir.z;
		s = -dir.y / dv;
		t = dir.x / dv;
	}

	vec2 uv = skyboxClampUv((vec2(s, t) + vec2(1.0)) * 0.5, skyboxFaceSize(axis));
	uv.y = 1.0 - uv.y;
	return uv;
}

vec3 sampleSkyboxFace(int face, vec2 uv)
{
	if (face == 0) {
		return texture(skyboxFace0, uv).rgb;
	}
	if (face == 1) {
		return texture(skyboxFace1, uv).rgb;
	}
	if (face == 2) {
		return texture(skyboxFace2, uv).rgb;
	}
	if (face == 3) {
		return texture(skyboxFace3, uv).rgb;
	}
	if (face == 4) {
		return texture(skyboxFace4, uv).rgb;
	}
	return texture(skyboxFace5, uv).rgb;
}

void shadeScene()
{
	vec3 base = (pushConstants.surfaceType > 0.5 || worldFlag(VK_WORLD_DRAWFLAT_COLOR))
		? pushConstants.color.rgb
		: max(inFlatColor, vec3(0.08));

	base = hdrMaterial(base);
	// True r_drawflat surfaces (not sky/turb, no fallback) still get shaded
	// by the surface's real lightmap, same as GLC/GLM's drawflat mode -- a
	// solid, completely unlit fill would otherwise flatten all depth cues.
	if (worldFlag(VK_WORLD_DRAWFLAT_COLOR) && pushConstants.surfaceType < 0.5) {
		base *= cvLighting(texture(lightmapTexture[0], inLightmapCoord).rgb, false);
	}

	if (pushConstants.surfaceType > 5.5) {
		if (pushConstants.useSkyTexture > 1.5) {
			vec3 dir = normalize(inDirection);
			int face = skyboxAxis(dir);

			base = hdrMaterial(sampleSkyboxFace(face, skyboxUv(face, dir)));
		}
		else if (pushConstants.useSkyTexture > 0.5) {
			const float len = 3.09375;
			vec3 dir = normalize(vec3(inDirection.x, inDirection.y, 3.0 * inDirection.z));
			float skySpeedscale = mod(pushConstants.time * 8.0, 128.0) / 128.0;
			float skySpeedscale2 = mod(pushConstants.time * 16.0, 128.0) / 128.0;
			vec2 skyCoord = vec2(skySpeedscale + dir.x * len, skySpeedscale + dir.y * len);
			vec2 cloudCoord = vec2(skySpeedscale2 + dir.x * len, skySpeedscale2 + dir.y * len);
			vec4 skyColour = texture(skyTexture, skyCoord);
			vec4 cloudColour = texture(skyCloudTexture, cloudCoord);

			base = mix(hdrMaterial(skyColour.rgb), hdrMaterial(cloudColour.rgb), cloudColour.a);
		}
	}

	fragColour = vec4(base, 1.0);
}

// HDR-001: emission follows the same coverage as color, independently of albedo.
layout(location=1) out vec4 fragEmission;
void main() {
 shadeScene();
 if(pushConstants.surfaceType<.5) fragColour.rgb += fragColour.rgb*shadowIrradiance();
 fragEmission=vec4(vec3(0), fragColour.a);
}
