/*
Copyright (C) 2014, Jeffrey Lim. All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

typedef union it_object_u it_object;
union it_object_u
{
	int typ;

	// 0: Box
	struct {
		int typ;
		int left, top, right, bottom;
		int style;
	} box;

	// 1: Text
	struct {
		int typ;
		int x, y;
		int color;
		const char *text;
	} text;

	// 2: Button
	struct {
		int typ;
		int a_up, a_down, a_left, a_right;
		int usage;
		int usage_radio_min, usage_radio_max;
		int button_effect;
		int eff_v1, eff_v2;
		void *eff_p1, *eff_p2;
		int left, top, right, bottom;
		int style;
		int flags;
		const char *text;
	} button;

	// 3: Empty
	// 4: Empty

	// 5: Select Direct Screen
	struct {
		int typ;
		int mode;
	} select_direct_screen;

	// 6: Redefine Characters
	struct {
		int typ;
		int first, num;
		const uint8_t *table;
	} redefine_characters;

	// 7: Empty

	// 8: Call Far Function
	struct {
		int typ;
		void *function;
	} call_far_function;

	// 9: Thumb bar
	struct {
		int typ;
		int x, y;
		int min_range, max_range;
		int wd1, wd2;
		int a_up, a_down, a_tab, a_shtab;
		int a_pgup, a_pgdn;
	} thumb_bar;

	// 10: Infoline
	struct {
		int typ;
		const char *text;
	} infoline;

	// 11: Set help context
	struct {
		int typ;
		int number;
	} set_help_context;

	// TODO: The rest
};

typedef struct it_objlist_2_s
{
	int typ;
	int unk1;
	void *ptrs[];
} it_objlist_2;

typedef struct it_objlist_6_s
{
	int typ;
	int unk1;
	void *ptrs[];
} it_objlist_6;

