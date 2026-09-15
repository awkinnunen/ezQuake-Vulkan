// HDR-001, OpenAI Codex, 2026-09-14. GPL-2.0-or-later.
#ifndef VK_HDR_GLSL
#define VK_HDR_GLSL
layout(constant_id=31) const bool hdrScene = false;
layout(constant_id=32) const bool hdrAdditive = false;
vec3 hdrDecode(vec3 c) {
 c=max(c,vec3(0));
 return mix(c/12.92,pow((c+.055)/1.055,vec3(2.4)),greaterThan(c,vec3(.04045)));
}
vec3 hdrEncode(vec3 c) {
 c=max(c,vec3(0));
 return mix(c*12.92,1.055*pow(c,vec3(1.0/2.4))-.055,greaterThan(c,vec3(.0031308)));
}
vec3 hdrMaterial(vec3 c) { return hdrScene ? hdrDecode(c) : c; }
#endif
