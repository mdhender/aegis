/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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

/**
  * The input_read function is used to read data from the given input
  * stream.  At most "size" bytes will be read from "ip" into "buffer".
  * The number of bytes actually read will be returned.  At end-of-file, a
  * value <=0 will be returned, and "buffer" will be unchanged.  All file
  * read errors or format errors are fatal, and will cause the function
  * to not return.
  */
long input_read _((input_ty *ip, void *buffer, size_t size));

/**
  * The input_read_strictest function is used to read data from the given input
  * stream.  Exactly "size" bytes will be read from "ip" into "buffer".
  * If there are less than "size" bytes available, a fatal error will result.
  */
void input_read_strictest _((input_ty *ip, void *buffer, size_t size));

/**
  * The input_read_strict function is used to read data from the
  * given input stream.  Exactly "size" bytes will be read from "ip"
  * into "buffer".  It is a fatal erros is less than "size" bytes are
  * available.  It returns zero at end of file, or non zero (but NOT
  * the number of bytes) if not at end of file.
  */
int input_read_strict _((input_ty *ip, void *buffer, size_t size));

/**
  * The input_skip function is used to read data from the given input
  * stream and discard it.  Exactly "size" bytes will be read from
  * "ip" into "buffer".  If there are less than "size" bytes available,
  * a fatal error will result.
  */
void input_skip _((input_ty *ip, size_t size));

/**
  * The input_getc_complicated function is used to get a character from
  * the input.  Usually users do not call this function directly, but
  * use the input_getc macro instead.
  */
int input_getc_complicated _((input_ty *));

/**
  * The input_ungetc_complicated function is used to push a character
  * back onto an input.  Usually users do not call this function directly,
  * but use the input_ungetc macro instead.
  */
void input_ungetc_complicated _((input_ty *, int));

/**
  * The input_unread function is used to return a block of charcatgres
  * to the input.  Think of it as a whole bunch of input_ungetc function
  * calls.
  */
void input_unread _((input_ty *, const void *, size_t));

/**
  * The input_delete function is usedto close the given input, and delete
  * all resources associated with it.  Once this returns, the given
  * input is no longer available for *any* use.
  */
void input_delete _((input_ty *));

/**
  * The input_ftell function is used to determine the current position
  * within the input.
  */
long input_ftell _((input_ty *));

/**
  * The input_fatal_error function is used to report a fatal error on
  * an input thream.  This function does not return.
  */
void input_fatal_error _((input_ty *, const char *));

struct output_ty; /* existence */

/**
  * The input_to_output function is used to copy the entire contents of
  * the inoput to the specified output.
  */
void input_to_output _((input_ty *, struct output_ty *));

/**
  * The input_one_line function is used to read one line from the input
  * (up to the next newline character or end of input).  The newline is
  * not included in the returned string.  Returns NULL if end-of-file
  * is reached.
  */
struct string_ty *input_one_line _((input_ty *));

/**
  * The input_name function is used to determine the name of the input.
  * (DO NOT use str_free when you are done with it.)
  */
#define input_name(fp) ((fp)->vptr->name(fp))

/**
  * The input_length function is used to determine the length of the
  * input.  May return -1 if the length is unknown.
  */
#define input_length(fp) ((fp)->vptr->length(fp))

/**
  * The input_getc function is used to get the next character from
  * the input.  Returns a value<=0 at end-of-file.
  */
#define input_getc(ip) \
	((ip)->buffer_position < (ip)->buffer_end ? \
	*((ip)->buffer_position)++ : \
	input_getc_complicated((ip)))

/**
  * The input_ungetc function is used to push back a character of input.
  * Think of it as undoing the effects of the input_getc function.
  */
#define input_ungetc(ip, c) \
	((ip)->buffer_position > (ip)->buffer ? \
	(void)(*(--((ip)->buffer_position)) = (c)) : \
	input_ungetc_complicated((ip), (c)))

#endif /* LIBAEGIS_INPUT_H */
