/* Config binding preview, OpenAI Codex, 2026-09-13. GPL-2.0-or-later.
 * Literal bind/unbind declarations only. Quake does not escape quotes with \.
 */
#include "cfg_bindings.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static int Equal(const char *a, const char *b)
{
	while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { a++; b++; }
	return *a == *b;
}

/* Tokens follow Quake's quoted-word convention, with // at token boundaries. */
static const char *Token(const char *p, char *out)
{
	int n = 0;
	while (*p && (unsigned char)*p <= ' ') p++;
	if (!*p || (p[0] == '/' && p[1] == '/')) return NULL;
	if (*p == '"') {
		p++;
		while (*p && *p != '"') out[n++] = *p++;
		if (*p) p++;
	} else {
		while (*p && (unsigned char)*p > ' ') out[n++] = *p++;
	}
	out[n] = 0;
	return p;
}

static void Command(cfg_bindings_t *out, char *line, int *physical,
	int (*key_number)(const char *, int), void (*set_binding)(cfg_bindings_t *, int, const char *))
{
	char cmd[CFG_BINDING_TEXT], key[CFG_BINDING_TEXT], word[CFG_BINDING_TEXT], value[CFG_BINDING_TEXT];
	const char *p = Token(line, cmd);
	int keynum, count = 0;
	size_t used = 0;
	if (!p) return;
	if (Equal(cmd, "unbindall")) {
		if (Token(p, word)) { out->warnings++; return; }
		memset(out->bindings, 0, sizeof(out->bindings));
		memset(out->declared, 1, sizeof(out->declared));
		return;
	}
	if (Equal(cmd, "con_bindphysical")) {
		p = Token(p, word);
		if (p && (!strcmp(word, "0") || !strcmp(word, "1"))) *physical = atoi(word) != 0;
		else if (p) out->warnings++;
		return;
	}
	if (!Equal(cmd, "bind") && !Equal(cmd, "unbind")) return;
	p = Token(p, key);
	if (!p) { out->warnings++; return; }
	if (strchr(key, '$')) { out->warnings++; return; }
	keynum = key_number(key, *physical);
	if (keynum < 0 || keynum >= CFG_BINDING_KEYS) { out->warnings++; return; }
	value[0] = 0;
	while ((p = Token(p, word)) != NULL) {
		size_t n = strlen(word);
		if (used + n + (count ? 1 : 0) >= sizeof(value)) { out->warnings++; return; }
		if (count++) value[used++] = ' ';
		memcpy(value + used, word, n + 1);
		used += n;
	}
	/* Inspect argument tokens, not trailing comments containing e.g. $names. */
	if (strchr(value, '$') || strchr(value, '{') || strchr(value, '}')) { out->warnings++; return; }
	if (Equal(cmd, "unbind")) {
		if (count) { out->warnings++; return; }
	} else if (!count) return; /* bind KEY is a query. */
	set_binding(out, keynum, value);
}

void CfgBindings_Parse(cfg_bindings_t *out, const char *text, size_t length,
	int (*key_number)(const char *, int), void (*set_binding)(cfg_bindings_t *, int, const char *))
{
	size_t pos = 0;
	int physical = 1;
	memset(out, 0, sizeof(*out));
	if (length >= 3 && !memcmp(text, "\xef\xbb\xbf", 3)) pos = 3;
	while (pos < length) {
		char line[CFG_BINDING_TEXT];
		size_t n = 0;
		int quoted = 0, comment = 0, braces = 0, overflow = 0;
		while (pos < length) {
			char c = text[pos++];
			if (!c) { out->warnings++; return; }
			if (c == '\n' && !braces) break;
			if (!comment && c == '"') quoted = !quoted;
			if (!comment && !quoted) {
				if (c == '{') braces++;
				if (c == '}' && braces) braces--;
				if (c == ';' && !braces) break;
				if (c == '/' && pos < length && text[pos] == '/') comment = 1;
			}
			if (c == '\n') comment = 0;
			if (c != '\r') {
				if (n < sizeof(line) - 1) line[n++] = c;
				else overflow = 1;
			}
		}
		line[n] = 0;
		if (overflow || quoted || braces) out->warnings++;
		else Command(out, line, &physical, key_number, set_binding);
	}
}
