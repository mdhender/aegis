//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2004 Peter Miller;
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
// MANIFEST: interface definition for common/mem.c
//

#ifndef COMMON_MEM_H
#define COMMON_MEM_H

/** \addtogroup Memory
  * \brief Memory management functions
  * \ingroup Common
  * @{
  */
#include <ac/stddef.h>
#include <main.h>

/**
  * \brief allocate memory
  *
  * mem_alloc uses malloc to allocate the required sized chunk of
  * memory.  If any error is returned from malloc() a fatal
  * diagnostic is issued.
  *
  * @param nbytes
  *     The size of the memory to allocate.
  *
  * @warning
  *     It is the responsibility of the caller to ensure that the space
  *     is freed when finished with, by a call to mem_free().
  */
void *mem_alloc(size_t nbytes);


/**
  * \brief allocate and clear memory
  *
  * mem_alloc_clear uses malloc to allocate the required sized chunk
  * of memory.  If any error is returned from malloc() an fatal
  * diagnostic is issued.  The memory is zeroed befor it is returned.
  *
  * @param nbytes
  *     The size of the memory to allocate
  *
  * @warning
  *     It is the responsibility of the caller to ensure that the space is
  *	freed when finished with, by a call to free().
  */
void *mem_alloc_clear(size_t nbytes);


void *mem_change_size(void *, size_t);
void mem_free(void *);

/**
  * The mem_copy_string function may be used to copy a C string into
  * dynamically allocated memory.
  *
  * \param arg
  *     The NUL terminated string to be copied.
  * \returns
  *     A copy of the string in dynamic memory.  Use mem_free when you
  *     are done with it.
  */
char *mem_copy_string(const char *arg);

/**
  * The mem_copy_string function may be used to copy a string into
  * dynamically allocated memory.
  *
  * \param arg
  *     The string to be copied (it is *not* NUL terminated).
  * \param len
  *     The length of the string to be copied.
  * \returns
  *     A copy of the string in dynamic memory.  The copy *is* NUL
  *     terminated.  Use mem_free when you are done with it.
  */
char *mem_copy_string(const char *arg, size_t len);

/** @} */
#endif // COMMON_MEM_H
