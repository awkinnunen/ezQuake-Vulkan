// OpenAI Codex, 2026-09-13. GPL-2.0-or-later.
vec2 skyboxClampUv(vec2 uv, ivec2 faceSize)
{
	// Preserve the original one-texel inset for 512x512 faces, per axis.
	// A 1- or 2-texel axis collapses to its centre, never inverted bounds.
	vec2 inset = min(vec2(0.5), 1.0 / vec2(max(faceSize, ivec2(1))));
	return clamp(uv, inset, vec2(1.0) - inset);
}
