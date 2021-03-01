//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 2002, 2003, 2005, 2006, 2008, 2012 Peter Miller.
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LEX_H
#define LEX_H

#include <common/nstring.h>

/**
  * The lex_open function is called to start lexical analysis of an
  * input file.
  *
  * @param filename
  *     The name of the file to be opened.
  */
void lex_open(const nstring &filename);

/**
  * The lex_close function is called to finish lexical analysis of an
  * input file.
  */
void lex_close(void);

/**
  * The parse_error function is used to report an error discovered
  * during the parsing of an input file.  (Called by the yacc-generated
  * grammar.)
  *
  * @param fmt
  *     The format controlling the output, see printf(3) for more
  *     information.
  */
void parse_error(const char *fmt, ...)                  ATTR_PRINTF(1, 2);

/**
  * The lex_message function is used to report some information
  * during the parse of a file.  The filename and line number will
  * automatically be included in the output.
  *
  * @param fmt
  *     The format controlling the output, see printf(3) for more
  *     information.
  */
void lex_message(const char *fmt, ...)                  ATTR_PRINTF(1, 2);

/**
  * The parse_lex function is used to obtain the next lexical token.
  * (Called by the yacc-generated parser.)
  */
int parse_lex(void);

/**
  * The lex_in_include_file method may be used to determine whether or
  * not the current parse point is within an include file or not.
  *
  * @returns
  *     bool; false if in top-level file, true in in include file or
  *     nested include file.
  */
bool lex_in_include_file(void);

/**
  * The lex_list_include_files function may be used to
  */
void lex_list_include_files(void);

/**
  * The elx_include_path function is sued to add another directory to
  * the end of the include search path.  May be called more than once.
  * Usually called by main() when processing command line options.
  *
  * @param dir
  *     The path of the directory to search in for include files.
  */
void lex_include_path(const nstring &dir);

/**
  * The lex_debug_printf function is used to print debug information
  * (called from the yacc-generated parser, indirectly).
  *
  * @param fmt
  *     The format controlling the output, see printf(3) for more
  *     information.
  */
void lex_debug_printf(const char *fmt, ...)             ATTR_PRINTF(1, 2);

/**
  * The lex_debug_fprintf function is used to print debug information
  * (called from the yacc-generated parser, indirectly).
  *
  * @param fp
  *     This parameter is ignored.
  * @param fmt
  *     The format controlling the output, see printf(3) for more
  *     information.
  */
void lex_debug_fprintf(void *fp, const char *fmt, ...)  ATTR_PRINTF(2, 3);

// fix for stupid Sun yacc
#define parse_error parse_error
#define parse_lex parse_lex

/**
  * The lex_comment_get function is used to fetch the most recently see
  * comment in the source file.  The is used to pass comments throgh
  * from the sourec file into the generated file's Doxygen comments.
  */
nstring lex_comment_get();

/**
  * The lex_location_get function is used to obtain a string describing
  * the current input file position.  This is used to insert the source
  * position into the output file.
  */
nstring lex_position_get();

#endif // LEX_H
// vim: set ts=8 sw=4 et :
