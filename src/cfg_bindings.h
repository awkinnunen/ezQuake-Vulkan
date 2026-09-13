/* Config binding preview, OpenAI Codex, 2026-09-13. GPL-2.0-or-later. */
#ifndef EZ_CFG_BINDINGS_H
#define EZ_CFG_BINDINGS_H

#include <stddef.h>
#define CFG_BINDING_KEYS 512
#define CFG_BINDING_TEXT 1024
typedef struct {
	char bindings[CFG_BINDING_KEYS][CFG_BINDING_TEXT];
	unsigned char declared[CFG_BINDING_KEYS];
	unsigned warnings;
} cfg_bindings_t;

/* Read-only: never executes commands, aliases or included files. */
void CfgBindings_Parse(cfg_bindings_t *out, const char *text, size_t length,
	int (*key_number)(const char *, int), void (*set_binding)(cfg_bindings_t *, int, const char *));
#endif
