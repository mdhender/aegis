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

#include <ac/stddef.h>

#include <main.h>

typedef struct input_ty input_ty;
struct input_ty
{
	struct input_vtbl_ty *vptr;
	/* private: */
	unsigned char	*buffer;
	size_t		buffer_size;
	unsigned char	*buffer_position;
	unsigned char	*buffer_end;
};

/*
 * This structure is *not* to be accessed by clients of this interface.
 * It is only present to permit optimizations.
 */
typedef struct input_vtbl_ty input_vtbl_ty;
struct input_vtbl_ty
{
	size_t size;
	void (*destruct)_((input_ty *));
	long (*read)_((input_ty *, void *, size_t));
	long (*ftell)_((input_ty *));
	struct string_ty *(*name)_((input_ty *));
	long (*length)_((input_ty *));
};

long input_read _((input_ty *, void *, size_t));
int input_getc_complicated _((input_ty *));
void input_ungetc_complicated _((input_ty *, int));
void input_unread _((input_ty *, const void *, size_t));
void input_delete _((input_ty *));
long input_ftell _((input_ty *));

void input_fatal_error _((input_ty *, const char *));

struct output_ty; /* existence */
void input_to_output _((input_ty *, struct output_ty *));
struct string_ty *input_one_line _((input_ty *));

#define input_name(fp) ((fp)->vptr->name(fp))
#define input_length(fp) ((fp)->vptr->length(fp))

#define input_getc(ip) \
	((ip)->buffer_position < (ip)->buffer_end ? \
	*((ip)->buffer_position)++ : \
	input_getc_complicated((ip)))
#define input_ungetc(ip, c) \
	((ip)->buffer_position > (ip)->buffer ? \
	(void)(*(--((ip)->buffer_position)) = (c)) : \
	input_ungetc_complicated((ip), (c)))

#endif /* LIBAEGIS_INPUT_H */
