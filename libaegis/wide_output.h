//
//	aegis - project change supervisor
//	Copyright (C) 1999-2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for libaegis/wide_output.c
//

#ifndef LIBAEGIS_WIDE_OUTPUT_H
#define LIBAEGIS_WIDE_OUTPUT_H

#include <common/main.h>

#include <common/ac/stddef.h>
#include <common/ac/wchar.h>

struct string_ty; // existence
struct wstring_ty; // existence

struct wide_output_ty
{
	struct wide_output_vtbl_ty *vptr;

	wchar_t		*buffer;
	size_t		buffer_size;
	wchar_t		*buffer_position;
	wchar_t		*buffer_end;

	size_t		ncallbacks;
	size_t		ncallbacks_max;
	struct wide_output_callback_record *callback;
};

//
// This structure is not to be used by clients of this API.  It is only
// present to permit macro optimization of the interface.
//
struct wide_output_vtbl_ty
{
	int		size;

	void (*destructor)(wide_output_ty *);
	struct string_ty *(*filename)(wide_output_ty *);
	void (*write)(wide_output_ty *, const wchar_t *, size_t);
	void (*flush)(wide_output_ty *);
	int (*page_width)(wide_output_ty *);
	int (*page_length)(wide_output_ty *);
	void (*end_of_line)(wide_output_ty *);

	//
	// By putting this last, we catch many cases where a method
	// pointer has been left out.
	//
	const char	*type_name;
};

typedef void (*wide_output_callback_ty)(wide_output_ty *, void *);

struct wide_output_callback_record
{
	wide_output_callback_ty func;
	void		*arg;
};

void wide_output_delete(wide_output_ty *);
struct string_ty *wide_output_filename(wide_output_ty *);
void wide_output_putwc(wide_output_ty *, wint_t);
void wide_output_putws(wide_output_ty *, const wchar_t *);
void wide_output_put_cstr(wide_output_ty *, const char *);
void wide_output_put_str(wide_output_ty *, struct string_ty *);
void wide_output_put_wstr(wide_output_ty *, struct wstring_ty *);
void wide_output_write(wide_output_ty *, const wchar_t *, size_t);
void wide_output_flush(wide_output_ty *);
int wide_output_page_width(wide_output_ty *);
int wide_output_page_length(wide_output_ty *);
void wide_output_end_of_line(wide_output_ty *);

void wide_output_delete_callback(wide_output_ty *, wide_output_callback_ty,
	void *);

//
// Despite looking recursive, it isn't.  Ansi C macros do not recurse,
// so it winds up calling the real function in the "buffer needs to
// grow" case.
//
// (Sun's compiler is broken, use GCC if you have a choice.)
//
#ifndef __SUNPRO_C
#define wide_output_putwc(fp, wc)					\
	(((fp)->buffer_position < (fp)->buffer_end)			\
	? (void)(*((fp)->buffer_position)++ = (wc))			\
	: wide_output_putwc((fp), (wc)))
#endif

#endif // LIBAEGIS_WIDE_OUTPUT_H
