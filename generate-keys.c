/******************************************************************
 * Copyright 2023-2024 Leonardo Hernández Hernández
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************/

/* cc -lxkbcommon -o generate-keys generate-keys.c */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

int
main(void)
{
	/* Allow generate keys with a different layout and variant.
	 * You can also use XKB_DEFAULT_* environmental variables and let this as is */
	struct xkb_rule_names rules = {
		0
	};
	struct xkb_context *context = NULL;
	struct xkb_keymap *keymap = NULL;
	xkb_keycode_t keycode, min_keycode, max_keycode;
	xkb_layout_index_t layout, num_layouts;
	xkb_level_index_t level, num_levels;
	int i, nsyms;
	const xkb_keysym_t *syms;
	char keyname[64];
	bool ok = false;
	FILE *file = fopen("keys.h", "w");
	if (!file) {
		perror("Couldn't open keys.h");
		return EXIT_FAILURE;
	}

	if (!(context = xkb_context_new(XKB_CONTEXT_NO_FLAGS))) {
		fputs("Couldn't create xkbcommon context\n", stderr);
		goto out;
	}

	if (!(keymap = xkb_keymap_new_from_names(context, &rules,
			XKB_KEYMAP_COMPILE_NO_FLAGS))) {
		fputs("Couldn't create xkbcommon keymap\n", stderr);
		goto out;
	}

	min_keycode = xkb_keymap_min_keycode(keymap);
	max_keycode = xkb_keymap_max_keycode(keymap);

	for (keycode = min_keycode; keycode <= max_keycode; keycode++) {
		num_layouts = xkb_keymap_num_layouts_for_key(keymap, keycode);
		for (layout = 0; layout < num_layouts; layout++) {
			num_levels = xkb_keymap_num_levels_for_key(keymap, keycode, layout);
			for (level = 0; level < num_levels; level++) {
				nsyms = xkb_keymap_key_get_syms_by_level(keymap, keycode, layout, level, &syms);
				for (i = 0; i < nsyms; i++) {
					xkb_keysym_get_name(syms[i], keyname, sizeof(keyname) / sizeof(keyname[0]));
					fprintf(file, "#define Key_%-27s %#05"PRIx32"\n", keyname, keycode);
				}
			}
		}
	}

	ok = true;
	sync();

out:
	fclose(file);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	return !ok;
}
