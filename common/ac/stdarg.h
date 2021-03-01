//
//	aegis - project change supervisor
//	Copyright (C) 1992-1994, 1996, 2002, 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef COMMON_AC_STDARG_H
#define COMMON_AC_STDARG_H

//
// This file contains insulation from whether <varargs.h> is being used
// or whether <stdarg.h> is being used.  Incompatibilities are hidden behind
// three macros:
//	sva_last	- last argument in variable arg func defn
//	sva_last_decl	- declaration for last arg
//	sva_start	- hides whether nth is used or not
// These macros are non-syntactic (ugh!) but they sure make things prettier.
//
// Do not directly include either <stdarg.h> or <varargs.h> in the code,
// always use this header <s-v-arg.h> instead.
//
// Never refer directly to va_start, va_arg, or va_dcl directly in the code.
// Direct references to va_list, va_arg and va_end are OK.
//

#include <common/config.h>
#include <stdarg.h>

#endif // COMMON_AC_STDARG_H
