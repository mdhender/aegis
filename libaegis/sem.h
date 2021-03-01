//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_SEM_H
#define LIBAEGIS_SEM_H

#include <common/nstring.h>

class meta_context; // forward
class input; // forward

/**
  * The sem_integer function is called by the parser when an integer
  * value is seen.
  *
  * @param n
  *     The value of the number.
  */
void sem_integer(long n);

/**
  * The sem_real function is called by the parser when a floating point
  * value is seen.
  *
  * @param n
  *     The value of the number.
  */
void sem_real(double n);

/**
  * The sem_string function is called by the parser when a string value
  * is seen.
  *
  * @param text
  *     The value of the string constant.
  */
void sem_string(const nstring &text);

/**
  * The sem_enum function is called when the parser sees a name in an
  * enumeration tag place.
  *
  * @param name
  *     The name of the enumerand.
  */
void sem_enum(const nstring &name);

/**
  * The sem_list function is called when the parser sees the start of a
  * list value.  The pop method will be called after the value has been
  * parsed.
  */
void sem_list(void);

/**
  * The sem_list_end function is called by the parser when it sees the
  * end of a list element.
  */
void sem_list_end(void);

/**
  * The sem_field function is called when the parser sees the start of
  * a name=value field.  The field_end method will be called after the
  * value has been parsed.
  *
  * @param name
  *     The nam eof the field.
  */
void sem_field(const nstring &name);

/**
  * The sem_field_end function is called by the parser when it sees the
  * end of a name=value field.
  */
void sem_field_end(void);

/**
  * The sem_parse function is used to parse the given input, a redirect
  * the parser actions through the given context.
  *
  * @param ctx
  *     The context for the parser's actions.
  * @param ifp
  *     The input stream to be parsed.
  */
void sem_parse(meta_context &ctx, input &ifp);

/**
  * The sem_parse_file function is used to parse the given file, and
  * redirect the parser actions through the given context.
  *
  * @param ctx
  *     The context for the parser's actions.
  * @param filename
  *     The name of the file to be parsed.
  */
void sem_parse_file(meta_context &ctx, const nstring &filename);

/**
  * The sem_parse_env function is used to parse the given environment
  * variable, and redirect the parser actions through the given context.
  *
  * @param ctx
  *     The context for the parser's actions.
  * @param name
  *     The name of the environment variable to be parsed.
  */
void sem_parse_env(meta_context &ctx, const nstring &name);

#endif // LIBAEGIS_SEM_H
