/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for libaegis/output.c
 */

#ifndef LIBAEGIS_OUTPUT_H
#define LIBAEGIS_OUTPUT_H

#include <ac/stdarg.h>
#include <ac/stddef.h>

#include <main.h>

struct string_ty; /* existence */
struct output_ty;

typedef void (*output_delete_callback_ty)_((struct output_ty *, void *));

typedef struct output_ty output_ty;
struct output_ty
{
	struct output_vtbl_ty *vptr;

	/* private: */
	output_delete_callback_ty del_cb;
	void		*del_cb_arg;

	/* private: */
	unsigned char	*buffer;
	size_t		buffer_size;
	unsigned char	*buffer_position;
	unsigned char	*buffer_end;
};

/*
 * This structure is not to be used by clients of this API.  It is only
 * present to permit macro optimization of the interface.
 */
typedef struct output_vtbl_ty output_vtbl_ty;
struct output_vtbl_ty
{
	int		size;

	void (*destructor)_((output_ty *));
	struct string_ty *(*filename)_((output_ty *));
	long (*ftell)_((output_ty *));
	void (*write)_((output_ty *, const void *, size_t));
	void (*flush)_((output_ty *));
	int (*page_width)_((output_ty *));
	int (*page_length)_((output_ty *));
	void (*eoln)_((output_ty *));

	/*
	 * By putting this last, we catch many cases where a method
	 * pointer has been left out.
	 */
	const char	*typename;
};

void output_delete _((output_ty *));
struct string_ty *output_filename _((output_ty *));
long output_ftell _((output_ty *));
void output_fputc _((output_ty *, int));
void output_fputs _((output_ty *, const char *));
void output_put_str _((output_ty *, struct string_ty *));
void output_write _((output_ty *, const void *, size_t));
void output_flush _((output_ty *));
int output_page_width _((output_ty *));
int output_page_length _((output_ty *));
void output_fprintf _((output_ty *, const char *, ...))
#ifdef __GNUC__
	__attribute__ ((__format__ (__printf__, 2, 3)))
#endif
		;
void output_vfprintf _((output_ty *, const char *, va_list));
void output_end_of_line _((output_ty *));
void output_delete_callback _((output_ty *, output_delete_callback_ty,
	void *));

/*
 * Despite looking recursive, it isn't.  Ansi C macros do not recurse,
 * so it winds up calling the real function in the "buffer needs to
 * grow" case.
 *
 * (Sun's compiler is defective, use GCC if you have a choice.)
 */
#ifndef __SUNPRO_C
#define output_fputc(fp, c) ((fp)->buffer_position < (fp)->buffer_end ? \
	(void)(*((fp)->buffer_position)++ = (c)) : output_fputc((fp), (c)))
#endif

#endif /* LIBAEGIS_OUTPUT_H */
