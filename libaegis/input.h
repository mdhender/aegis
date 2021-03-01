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
 * MANIFEST: interface definition for libaegis/input.c
 */

#ifndef LIBAEGIS_INPUT_H
#define LIBAEGIS_INPUT_H

#include <main.h>

typedef struct input_ty input_ty;
struct input_ty
{
	struct input_vtbl_ty *vptr;
	/* private: */
	unsigned char *pushback_buf;
	int pushback_len;
	int pushback_max;
};

/*
 * This structure is *not* to be accessed by clients of this interface.
 * It is only present to permit optimizations.
 */
typedef struct input_vtbl_ty input_vtbl_ty;
struct input_vtbl_ty
{
	int size;
	void (*destruct)_((input_ty *));
	long (*read)_((input_ty *, void *, long));
	int (*get)_((input_ty *));
	long (*ftell)_((input_ty *));
	const char *(*name)_((input_ty *));
	long (*length)_((input_ty *));
};

long input_read _((input_ty *, void *, long));
int input_getc _((input_ty *));
void input_ungetc _((input_ty *, int));
const char *input_name _((input_ty *));
void input_delete _((input_ty *));
long input_length _((input_ty *));
long input_ftell _((input_ty *));

void input_format_error _((input_ty *));

struct output_ty; /* existence */
void input_to_output _((input_ty *, struct output_ty *));
struct string_ty *input_one_line _((input_ty *));

#ifdef __GNUC__
extern __inline long input_read(input_ty *fp, void *data, long len)
	{ if (len <= 0) return 0; if (fp->pushback_len > 0) {
	fp->pushback_len--; *(char *)data = fp->pushback_buf[
	fp->pushback_len ]; return 1; } return fp->vptr->read(fp, data, len); }
extern __inline int input_getc(input_ty *fp) { if (fp->pushback_len >
	0) { fp->pushback_len--; return fp->pushback_buf[ fp->pushback_len
	]; } return fp->vptr->get(fp); }
extern __inline const char *input_name(input_ty *fp)
	{ return fp->vptr->name(fp); }
extern __inline long input_length(input_ty *fp)
	{ return fp->vptr->length(fp); }
extern __inline long input_ftell(input_ty *fp)
	{ return fp->vptr->ftell(fp) - fp->pushback_len; }
#else /* !__GNUC__ */
#ifdef DEBUG
#define input_name(fp) ((fp)->vptr->name(fp))
#define input_length(fp) ((fp)->vptr->length(fp))
#define input_ftell(fp) ((fp)->vptr->ftell(fp) - (fp)->pushback_len)
#endif /* DEBUG */
#endif /* !__GNUC__ */

#endif /* LIBAEGIS_INPUT_H */
