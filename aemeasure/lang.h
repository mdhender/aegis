//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
//	All rights reserved.
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
// MANIFEST: interface definition for aemeasure/lang.c
//

#ifndef AEMEASURE_LANG_H
#define AEMEASURE_LANG_H

#include <ac/stdio.h>


struct lang_data_t
{
    long            code_lines;
    long            comment_lines;
    long            blank_lines;
    int		    file_is_binary;
    long            length;

    int             line_has_code;
    int             line_has_comment;
    int             line_length;

    long            halstead_n1;
    long            halstead_n2;
    long            halstead_N1;
    long            halstead_N2;
};


typedef int (*lang_test_t)(const char *);
typedef void (*lang_count_t)(FILE *, lang_data_t *);

struct lang_t
{
    const char      *name;
    lang_count_t    count;
    lang_test_t     test;
};


lang_t *lang_by_name(const char *name);
lang_t *lang_by_file(const char *filename);
void lang_data_print(FILE *, lang_data_t *);

#endif // AEMEASURE_LANG_H
