#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_hdr.glsl"
#extension GL_GOOGLE_include_directive : require
#define CV_SET 1
#include "vk_competitive.glsl"

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 fragColour;

layout(binding = 0) uniform sampler2D sceneColor;
layout(binding = 1) uniform sampler2D sceneEmission;

layout(push_constant) uniform PushConstants {
	vec4 blend;     // damage/pickup/underwater tint, premultiplied like v_blend
	float gamma;
	float contrast;
	float invWidth;
	float invHeight;
	int fxaaEnabled;
	float fxaaQuality; // 0 = off; otherwise 0..1, see VK_FxaaQualityFromPreset
	int bloomSource;
} pc;

vec3 hdrTone(vec3 linear) {
 linear=max(linear*cv.post.x,vec3(0));
 if(cv.post.y>.5 && cv.post.y<1.5) linear=linear/(vec3(1)+linear*.35);
 else if(cv.post.y>1.5) linear=clamp((linear*(2.51*linear+.03))/(linear*(2.43*linear+.59)+.14),0.0,1.0);
 return hdrEncode(linear);
}
vec3 aaSample(vec2 uv) {
 vec3 c=texture(sceneColor,uv).rgb;
 return hdrScene ? hdrTone(c) : c;
}

// Cheap edge-detect AA: blends towards the average of the 4 diagonal taps on
// high-contrast edges. Not the full NVIDIA FXAA 3.11 quality search (that
// header assumes a GLSL-text include pipeline; this engine embeds Vulkan
// shaders as precompiled SPIR-V), but same green-as-luma edge metric and same
// "only touch actual edges" behaviour, at a fraction of the ALU cost --
// reasonable for a mobile-first post-process pass.
//
// vid_framebuffer_fxaa (0-17) picks one of 17 real FXAA_QUALITY__PRESET
// values on GLC/GLM (see GL_FramebufferFxaaPreset), each of which compiles a
// different NVIDIA FXAA 3.11 variant with its own edge threshold/subpixel
// search depth. This single-pass approximation doesn't have per-preset
// shader variants to select between, so instead of collapsing the whole
// range to a single on/off (as it did before), quality maps continuously
// onto the two knobs this algorithm actually has: a lower edge threshold
// (more edges get touched, matching higher-quality presets being more
// sensitive) and a stronger blend cap (closer to full antialiasing on the
// edges it does find).
vec3 ApplyFXAA(vec3 centerColor)
{
	vec2 rcpFrame = vec2(pc.invWidth, pc.invHeight);
	vec3 nw = aaSample(texCoord + vec2(-1.0, -1.0) * rcpFrame);
	vec3 ne = aaSample(texCoord + vec2( 1.0, -1.0) * rcpFrame);
	vec3 sw = aaSample(texCoord + vec2(-1.0,  1.0) * rcpFrame);
	vec3 se = aaSample(texCoord + vec2( 1.0,  1.0) * rcpFrame);

	float lumaM = centerColor.g;
	float lumaNW = nw.g;
	float lumaNE = ne.g;
	float lumaSW = sw.g;
	float lumaSE = se.g;

	float rangeMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float rangeMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	float range = rangeMax - rangeMin;

	// 0.1000 (least sensitive) down to 0.0500 (most sensitive) as quality
	// goes 0..1 -- FXAA_EDGE_THRESHOLD_MIN across the real presets spans
	// roughly this range.
	float edgeThreshold = mix(0.1000, 0.0500, pc.fxaaQuality);
	if (range < edgeThreshold) {
		return centerColor;
	}

	vec3 average = (nw + ne + sw + se + centerColor) * 0.2;
	// 0.50 (subtle) up to 1.00 (full strength) as quality goes 0..1.
	float maxBlend = mix(0.50, 1.00, pc.fxaaQuality);
	float blendAmount = clamp(range * 4.0, 0.0, maxBlend);
	return mix(centerColor, average, blendAmount);
}

// Small normalized kernel: bright scene pixels only, never HUD or crosshair.
vec3 cvBloom() {
 vec3 glow=vec3(0.0); float weights=0.0;
 vec2 pixel=vec2(pc.invWidth,pc.invHeight)*cv.post2.x;
 for(int y=-2;y<=2;++y) for(int x=-2;x<=2;++x) {
  float w=exp(-float(x*x+y*y)*.65);
  vec3 sampleColour=texture(sceneColor,texCoord+vec2(x,y)*pixel).rgb;
  if(hdrScene && pc.bloomSource!=0) sampleColour=texture(sceneEmission,texCoord+vec2(x,y)*pixel).rgb;
  float peak=max(sampleColour.r,max(sampleColour.g,sampleColour.b));
  float contribution=max(0.0,peak-cv.post.w)/max(peak,.001);
  glow+=sampleColour*contribution*w; weights+=w;
 }
 return glow/max(weights,.001)*cv.post.z;
}
vec3 cvTone(vec3 colour) {
 if(cv.post.y<.5 && abs(cv.post.x-1.0)<.0001) return colour;
 vec3 linear=pow(max(colour,vec3(0)),vec3(2.2))*cv.post.x;
 if(cv.post.y>.5 && cv.post.y<1.5) linear=linear/(vec3(1)+linear*.35);
 else if(cv.post.y>1.5) linear=clamp((linear*(2.51*linear+.03))/(linear*(2.43*linear+.59)+.14),0.0,1.0);
 return pow(max(linear,vec3(0)),vec3(1.0/2.2));
}

void main()
{
	vec3 colour = texture(sceneColor, texCoord).rgb;
	if (hdrScene) {
		// Float scene and bloom stay linear until the output tone curve.
		if(cv.post.z>0.0) colour+=cvBloom();
		colour=hdrTone(colour);
	}

	if (pc.fxaaEnabled != 0) {
		colour = ApplyFXAA(colour);
	}

	if(cv.post2.w>0.0) {
  vec2 p=vec2(pc.invWidth,pc.invHeight);
  vec3 a=aaSample(texCoord+vec2(p.x,0));
  vec3 b=aaSample(texCoord-vec2(p.x,0));
  vec3 c=aaSample(texCoord+vec2(0,p.y));
  vec3 d=aaSample(texCoord-vec2(0,p.y));
  vec3 lo=min(colour,min(min(a,b),min(c,d))), hi=max(colour,max(max(a,b),max(c,d)));
  colour=clamp(colour+(colour-(a+b+c+d)*.25)*cv.post2.w,lo,hi);
 }
 if(!hdrScene) {
  if(cv.post.z>0.0) colour+=cvBloom();
  colour=cvTone(colour);
 }

 // Same formula as src/glsl/post_process_screen.fragment.glsl
	// (EZ_POSTPROCESS_PALETTE path): blend/tint, then contrast, then gamma.
	colour = (colour * pc.blend.a + pc.blend.rgb) * pc.contrast;
	colour = pow(max(colour, vec3(0.0)), vec3(pc.gamma));

	fragColour = vec4(colour, 1.0);
}
