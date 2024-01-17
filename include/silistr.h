/*
    code based on sili.h
    https://github.com/EimaMei/sili/

    ------------------------------------------------------------------------------
    Copyright (C) 2023 EimaMei

    This software is provided 'as-is', without any express or implied warranty. In
    no event will the authors be held liable for any damages arising from the use of
    this software.

    Permission is granted to anyone to use this software for any purpose, including
    commercial applications, and to alter it and redistribute it freely, subject to
    the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
    ------------------------------------------------------------------------------
*/

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

typedef char* cstring;
typedef size_t usize;
typedef ptrdiff_t isize;
typedef void* rawptr;

typedef char* siString;
typedef struct { size_t len, capacity; } siStringHeader;
#define SI_STRING_HEADER(str) ((siStringHeader*)str - 1)

#define si_swap(a, b) do { typeof((a)) tmp = (a); (a) = (b); (b) = tmp; } while (0)
#define si_abs(x) ((x) < 0 ? -(x) : (x))
#define si_between(x, lower, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define si_pause() do { printf("Press any key to continue...\n"); getchar(); } while(0)

siString si_string_make_reserve(usize len) {
	rawptr ptr = malloc(sizeof(siStringHeader) + (len + 1));

	siString res_str = (siString)ptr + sizeof(siStringHeader);
	res_str[len] = '\0';

	siStringHeader* header = SI_STRING_HEADER(res_str);
	header->len = 0;
	header->capacity = len;
	/*header->type_size = sizeof(char); */

	return res_str;
}

siString si_string_make_len(cstring str, usize len) {
	siString res_str = si_string_make_reserve(len);
	memcpy(res_str, str, len);

	siStringHeader* header = SI_STRING_HEADER(res_str);
	header->len = len;

	return res_str;
}

void si_string_make_space_for(siString* str, usize add_len) {
	siStringHeader* header = SI_STRING_HEADER(*str);
	usize old_size = sizeof(siStringHeader) + (header->len + 1 * sizeof(char));
	usize new_size = old_size + add_len;

	siStringHeader* new_header = (siStringHeader*)realloc(header, new_size);

    *str = (char*)new_header + sizeof(siStringHeader);
	new_header->capacity += add_len;
}


usize si_string_len(siString str);
inline usize si_string_len(siString str) {
	return SI_STRING_HEADER(str)->len;
}

void si_string_reverse_len(siString* str, usize len) {
	siString actual_str = *str;

	char* a = actual_str;
	char* b = actual_str + len - 1;
	len *= 0.5;

	while (len--) {
		si_swap(*a, *b);
		a++, b--;
	}

	*str = actual_str;
}

void si_string_append_len(siString* str, cstring other, usize other_len) {
	siString cur_str = *str;

	siStringHeader* header = SI_STRING_HEADER(cur_str);
	usize previous_len = header->len;
	header->len += other_len;

	if (header->capacity < header->len) {
        si_string_make_space_for(str, other_len);
		cur_str = *str;
		header = SI_STRING_HEADER(cur_str);
	}

	memcpy(cur_str + previous_len, other, other_len);
	cur_str[header->len] = '\0';
}


void si_string_append(siString* str, cstring other) {
	si_string_append_len(str, other, strlen(other));
}

void si_string_free(siString str);
inline void si_string_free(siString str) {
	if (str == NULL) {
		return ;
	}
	free(SI_STRING_HEADER(str));
}

u64 si_cstr_to_u64(cstring str) {
	u64 result = 0;
	char cur;
	while ((cur = *str++)) {
		if (cur >= '0' && cur <= '9') {
			result = (result * 10) + (cur - '0');
		}
	}

	return result;
}