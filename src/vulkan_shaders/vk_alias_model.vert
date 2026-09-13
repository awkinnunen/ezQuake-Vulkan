#version 450
#extension GL_GOOGLE_include_directive : enable
#define CV_SET 1
#include "vk_competitive.glsl"
#extension GL_GOOGLE_include_directive : require
#include "vk_alias_lighting.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inDirection;

layout(push_constant) uniform PushConstants {
	mat4 mvp;
	vec4 color;
	vec4 altColor;
	float lerp;
	float textured;
	float weapon;
	float mode;
	float minLumaMix;
	float scrollS;
	float scrollT;
	float textureIndex;
} pushConstants;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec4 outColor;
layout(location = 2) out float outTextured;
layout(location = 3) out vec2 outAltTexCoord;
layout(location = 4) out vec4 outAltColor;
layout(location = 5) out float outMode;
layout(location = 6) out float outMinLumaMix;
layout(location = 7) out vec3 cvNormal;
layout(location = 8) out vec3 cvPosition;
layout(location = 9) out float cvUp;

void main()
{
	vec3 position = inPosition + inDirection * pushConstants.lerp;

	if (pushConstants.mode > 3.5) {
		position += inNormal * pushConstants.altColor.x;
	}
	else if (pushConstants.mode > 2.5) {
		position.x -= pushConstants.altColor.x * (position.z + pushConstants.altColor.z);
		position.y -= pushConstants.altColor.y * (position.z + pushConstants.altColor.z);
		position.z = 1.0 - pushConstants.altColor.z;
	}
	else if (pushConstants.mode > 1.5) {
		position += inNormal * 0.5;
	}

	vec4 clip = pushConstants.mvp * vec4(position, 1.0);

	clip.y = -clip.y;
	if (pushConstants.weapon > 0.5) {
		clip.z *= 0.3;
	}
	clip.z = clip.z * 0.5 + clip.w * 0.5;

	gl_Position = clip;
	cvPosition = cvNormal = vec3(0);
	cvUp = 0;
	// Uniform per-draw branch: ordinary rendering and powerup passes do not
	// pay for a normal-matrix inverse when the surface rim is disabled.
	if (cv.rim.x > 0.0 && pushConstants.mode < 0.5) {
		cvPosition = (cv.modelView * vec4(position,1)).xyz;
		cvNormal = transpose(inverse(mat3(cv.modelView))) * inNormal;
		cvUp = inNormal.z;
	}
	if (pushConstants.mode > 3.5) {
		outTexCoord = inTexCoord;
		outAltTexCoord = inTexCoord;
	}
	else if (pushConstants.mode > 2.5) {
		outTexCoord = inTexCoord;
		outAltTexCoord = inTexCoord;
	}
	else if (pushConstants.mode > 1.5) {
		vec2 scroll = vec2(pushConstants.scrollS, pushConstants.scrollT);
		outTexCoord = inTexCoord * 2.0 + scroll;
		outAltTexCoord = inTexCoord * 2.0 - scroll;
	}
	else {
		outTexCoord = inTexCoord;
		outAltTexCoord = inTexCoord;
	}
	outColor = aliasLitColor(pushConstants.color, inNormal, pushConstants.altColor.xyz, pushConstants.mode);
	outAltColor = pushConstants.altColor;
	outTextured = pushConstants.textured;
	outMode = pushConstants.mode;
	outMinLumaMix = pushConstants.minLumaMix;
}
