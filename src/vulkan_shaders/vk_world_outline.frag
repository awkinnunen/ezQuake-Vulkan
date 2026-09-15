#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_hdr.glsl"

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 fragColour;

layout(binding = 0) uniform sampler2D normalTexture;

layout(push_constant) uniform PushConstants {
	vec3 outlineColor;
	float outlineScale;
	float outlineDepthThreshold;
	float outlineNormalThreshold;
	float invWidth;
	float invHeight;
	float zFar;
	float aoStrength;
	float aoRadius;
	float edges;
 float edgeOpacity;
 // Explicit offsets match the bounded 112-byte C block.
 layout(offset=64) vec4 rayRight;
 vec4 rayUp;
 vec4 rayForward;
} pc;

// SSAO-001, OpenAI Codex: camera-relative world positions, not raw depth
// differences. Hemisphere bias rejects coplanar self-occlusion; radial range
// weighting rejects unrelated foreground surfaces and invalid sky/water samples.
vec3 positionAt(vec2 uv, float distanceFromCamera) {
 vec3 ray=pc.rayForward.xyz+pc.rayRight.xyz*(uv.x*2.0-1.0)+pc.rayUp.xyz*(1.0-uv.y*2.0);
 return normalize(ray)*distanceFromCamera;
}
float ssaoContribution(vec3 normal, vec3 delta, float radius, float bias) {
 float d=length(delta);
 if(d<.001 || d>=radius) return 0.0;
 float hemisphere=max(0.0,dot(normal,delta)-bias)/d;
 return hemisphere*(1.0-smoothstep(radius*.25,radius,d));
}
float ssao(vec4 center) {
 vec3 position=positionAt(texCoord,center.a*pc.zFar);
 vec3 normal=normalize(center.rgb);
 if(dot(normal,-position)<0.0) normal=-normal;
 vec2 focal=.5/vec2(length(pc.rayRight.xyz),length(pc.rayUp.xyz));
 vec2 radius=pc.aoRadius/max(length(position),1.0)*focal;
 int samples=clamp(int(pc.rayRight.w),8,32);
 float occlusion=0.0;
 for(int i=0;i<32;++i) {
  if(i>=samples) break;
  float angle=float(i)*2.39996323;
  float scale=sqrt((float(i)+.5)/float(samples));
  vec2 uv=texCoord+vec2(cos(angle),sin(angle))*radius*scale;
  if(any(lessThan(uv,vec2(0)))||any(greaterThan(uv,vec2(1)))) continue;
  vec4 neighbour=texture(normalTexture,uv);
  if(neighbour.a<=0.0) continue;
  vec3 delta=positionAt(uv,neighbour.a*pc.zFar)-position;
  occlusion+=ssaoContribution(normal,delta,pc.aoRadius,pc.rayForward.w);
 }
 return clamp(occlusion/float(samples)*4.0*pc.aoStrength,0.0,pc.aoStrength);
}

// Port of GLM's fx_world_geometry.fragment.glsl -- see that file for the
// original. Same finite-difference edge test: a real normal discontinuity
// (vec_nequ) always draws an outline; otherwise a second-derivative depth
// jump (the "kink" a corner between two coplanar-looking but distant
// surfaces produces) also counts.
bool vec_nequ(vec3 a, vec3 b)
{
	return dot(a, b) < pc.outlineNormalThreshold;
}

void drawEdges()
{
	vec2 offset = vec2(pc.outlineScale * pc.invWidth, pc.outlineScale * pc.invHeight);

	vec4 center = texture(normalTexture, texCoord);
	vec4 left   = texture(normalTexture, texCoord - vec2(offset.x, 0.0));
	vec4 right  = texture(normalTexture, texCoord + vec2(offset.x, 0.0));
	vec4 up     = texture(normalTexture, texCoord - vec2(0.0, offset.y));
	vec4 down   = texture(normalTexture, texCoord + vec2(0.0, offset.y));

	bool ignore = center.a == left.a && center.a == right.a && center.a == up.a && center.a == down.a;
	if (ignore || center.a == 0.0) {
		fragColour = vec4(0.0);
		return;
	}

	if ((left.a  != 0.0 && vec_nequ(center.rgb, left.rgb )) ||
	    (right.a != 0.0 && vec_nequ(center.rgb, right.rgb)) ||
	    (up.a    != 0.0 && vec_nequ(center.rgb, up.rgb   )) ||
	    (down.a  != 0.0 && vec_nequ(center.rgb, down.rgb ))) {
		fragColour = vec4(hdrMaterial(pc.outlineColor), 1.0);
		return;
	}

	bool zDiffH = pc.zFar * abs((right.a - center.a) - (center.a - left.a)) > pc.outlineDepthThreshold;
	bool zDiffV = pc.zFar * abs((down.a - center.a) - (center.a - up.a)) > pc.outlineDepthThreshold;

	if (center.a != 0.0 && (
	    (left.a != 0.0 && right.a != 0.0 && zDiffH) ||
	    (down.a != 0.0 && up.a    != 0.0 && zDiffV))) {
		fragColour = vec4(hdrMaterial(pc.outlineColor), 1.0);
		return;
	}

	fragColour = vec4(0.0);
}

// Non-temporal world contact shading. Same-plane samples are excluded so a
// sloping wall cannot darken itself. Depth discontinuities beyond the radius
// and sky/water sentinels never occlude. Drawn before players and the HUD.
void shadeScene() {
 vec4 center=texture(normalTexture,texCoord);
 float ao=0.0;
 if(pc.aoStrength>0.0 && center.a>0.0) {
  if(pc.rayRight.w>0.0) ao=ssao(center);
  else {
  float depth=center.a*pc.zFar;
  float pixels=clamp(pc.aoRadius/max(depth,1.0)/pc.invHeight*.5,1.0,40.0);
  for(int i=0;i<8;++i) {
   float angle=float(i)*.7853981634;
   vec2 offset=vec2(cos(angle),sin(angle))*vec2(pc.invWidth,pc.invHeight)*pixels;
   vec4 neighbour=texture(normalTexture,texCoord+offset);
   float dz=depth-neighbour.a*pc.zFar;
   float bend=1.0-smoothstep(.65,.98,dot(center.rgb,neighbour.rgb));
   if(neighbour.a>0.0 && dz>0.0 && dz<pc.aoRadius)
    ao+=bend*smoothstep(0.0,pc.aoRadius*.2,dz)*(1.0-dz/pc.aoRadius);
  }
  ao=clamp(ao/8.0*pc.aoStrength*2.0,0.0,pc.aoStrength);
  }
 }
 fragColour=vec4(0);
 if(pc.edges>.5) { drawEdges(); fragColour.a *= pc.edgeOpacity; }
 if(fragColour.a==0.0) fragColour=vec4(0,0,0,ao);
}

// HDR-001: emission follows the same coverage as color, independently of albedo.
layout(location=1) out vec4 fragEmission;
void main() {
 shadeScene();
 fragEmission=vec4(vec3(0), 0.0);
}
