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

typedef struct output_ty output_ty;
struct output_ty
{
	struct output_vtbl_ty *vptr;
};

/*
 * This structure is not to be used by clients of this API.  It is only
 * present to permit macro optimization of the interface.
 */
typedef struct output_vtbl_ty output_vtbl_ty;
struct output_vtbl_ty
{
	int		size;
	const char	*typename;

	void (*destructor)_((output_ty *));
	const char *(*filename)_((output_ty *));
	long (*ftell)_((output_ty *));
	void (*fputc)_((output_ty *, int));
	void (*fputs)_((output_ty *, const char *));
	void (*write)_((output_ty *, const void *, size_t));
};

void output_delete _((output_ty *));
const char *output_filename _((output_ty *));
long output_ftell _((output_ty *));
void output_fputc _((output_ty *, int));
void output_fputs _((output_ty *, const char *));
void output_write _((output_ty *, const void *, size_t));
void output_fprintf _((output_ty *, const char *, ...));
void output_vfprintf _((output_ty *, const char *, va_list));

#ifdef __GNUC__

extern __inline const char *output_filename(output_ty *fp) { return
	fp->vptr->filename(fp); }
extern __inline long output_ftell(output_ty *fp) { return
	fp->vptr->ftell(fp); }
extern __inline void output_fputc(output_ty *fp, int c) {
	fp->vptr->fputc(fp, c); }
extern __inline void output_fputs(output_ty *fp, const char *s) {
	fp->vptr->fputs(fp, s); }
extern __inline void output_write(output_ty *fp, const void *data, size_t len) {
	fp->vptr->write(fp, data, len); }

#else /* !__GNUC__ */

#ifndef DEBUG
#define output_filename(fp) ((fp)->vptr->filename(fp))
#define output_ftell(fp) ((fp)->vptr->ftell(fp))
#define output_fputc(fp, c) ((fp)->vptr->fputc((fp), (c)))
#define output_fputs(fp, s) ((fp)->vptr->fputs((fp), (s)))
#define output_write(fp, data, len) ((fp)->vptr->write((fp), (data), (len)))
#endif /* DEBUG */

#endif /* __GNUC__ */

#endif /* LIBAEGIS_OUTPUT_H */
