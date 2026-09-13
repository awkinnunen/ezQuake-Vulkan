// Adapted by OpenAI Codex, 2026-09-13, from the existing ezQuake GLM shader
// draw_aliasmodel.vertex.glsl. GPL-2.0-or-later. Original approximation credits:
// mh @ http://forums.insideqc.com/viewtopic.php?f=3&t=2983
// lighting = (ambientlight, shadelight, yaw in radians).
vec4 aliasLitColor(vec4 color, vec3 normal, vec3 lighting, float mode)
{
	// Only the normal material pass uses directional lighting. Preserve alpha
	// and GLM's shadelight >= 1000 fullbright sentinel exactly.
	if (mode > 0.5 || lighting.y >= 1000.0) {
		return color;
	}
	vec3 angleVector = normalize(vec3(cos(-lighting.z), sin(-lighting.z), 1.0));
	float light = floor((dot(normal, angleVector) + 1.0) * 127.0) / 127.0;
	light = min((light * lighting.y + lighting.x) / 256.0, 1.0);
	return vec4(color.rgb * light, color.a);
}
