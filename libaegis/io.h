//
//	aegis - project change supervisor
//	Copyright (C) 1991-1996, 1999, 2002-2006 Peter Miller
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
// MANIFEST: interface definition for aegis/io.c
//

#ifndef LIBAEGIS_IO_H
#define LIBAEGIS_IO_H

#include <common/ac/time.h>

#include <common/main.h>
#include <libaegis/meta_type.h>

struct sub_context_ty; // forward
struct output_ty; // forward
class nstring; // forward

#define INTEGER_NOT_SET 0
#define REAL_NOT_SET 0.0
#define TIME_NOT_SET (time_t)0

void boolean_write(output_ty *, const char *, bool, int);
void boolean_write_xml(output_ty *, const char *, bool, int);

/**
  * The string_write function is used to write aout a name value pair
  * to the given output stream, in a format suitable for reading by the
  * meta-date parser.
  *
  * @param op
  *     The output stream to write on.
  * @param name
  *     The name of the field.
  * @param value
  *     The value of the field.
  */
void string_write(output_ty *op, const char *name, string_ty *value);

/**
  * The string_write function is used to write aout a name value pair
  * to the given output stream, in a format suitable for reading by the
  * meta-date parser.
  *
  * @param op
  *     The output stream to write on.
  * @param name
  *     The name of the field.
  * @param value
  *     The value of the field.
  */
void string_write(output_ty *op, const char *name, const nstring &value);

/**
  * The string_write _xmlfunction is used to write out an XML element
  * and its value.
  *
  * @param op
  *     The output stream to write on.
  * @param name
  *     The name of the elsement.
  * @param value
  *     The value of the elsement.
  */
void string_write_xml(output_ty *op, const char *name, string_ty *value);

/**
  * The string_write_xml function is used to write out an XML element
  * and its value.
  *
  * @param op
  *     The output stream to write on.
  * @param name
  *     The name of the elsement.
  * @param value
  *     The value of the elsement.
  */
void string_write_xml(output_ty *op, const char *name, const nstring &value);

void integer_write(output_ty *, const char *, long, int);
void integer_write_xml(output_ty *, const char *, long, int);
void real_write(output_ty *, const char *, double, int);
void real_write_xml(output_ty *, const char *, double, int);
void time_write(output_ty *, const char *, time_t, int);
void time_write_xml(output_ty *, const char *, time_t, int);
void io_comment_append(sub_context_ty *, const char *);
void io_comment_emit(output_ty *);

#endif // LIBAEGIS_IO_H
