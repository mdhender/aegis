//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate dumps
//

#include <error.h>
#include <symtab.h>


//
// NAME
//	symtab_dump - dump id table
//
// SYNOPSIS
//	void symtab_dump(symtab_ty *stp, char *caption);
//
// DESCRIPTION
//	The symtab_dump function is used to dump the contents of the
//	symbol table.  The caption will be used to indicate why the
//	symbol table was dumped.
//
// CAVEAT
//	This function is only available when symbol DEBUG is defined.
//

void
symtab_dump(symtab_ty *stp, char *caption)
{
    str_hash_ty     j;
    symtab_row_ty   *p;

    error_raw("symbol table %s = {", caption);
    for (j = 0; j < stp->hash_modulus; ++j)
    {
	for (p = stp->hash_table[j]; p; p = p->overflow)
	{
	    error_raw
	    (
	       	"key = \"%s\", data = %08lX",
	       	p->key->str_text,
	       	(long)p->data
	    );
	}
    }
    error_raw("}");
}
