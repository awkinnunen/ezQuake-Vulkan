#version 450
#extension GL_GOOGLE_include_directive : require
#include "vk_shadow_face.glsl"
layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 direction;
layout(location=0) out vec2 texCoord;
layout(push_constant) uniform ShadowPush {mat4 world;vec4 light;vec4 params;vec4 direction;} pc;
void main() {
 vec3 d=(pc.world*vec4(position+direction*pc.params.w,1)).xyz-pc.light.xyz;
 vec3 q=pc.direction.w>0?shadowSpot(d,pc.direction):shadowFace(d,int(pc.params.x));
 float a=pc.light.w/(pc.light.w-1.0);
 gl_Position=vec4(q.xy,a*q.z-a,q.z);
 texCoord=uv;
}
