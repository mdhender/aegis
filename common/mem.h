/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1999, 2004 Peter Miller;
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
 * MANIFEST: interface definition for common/mem.c
 */

#ifndef MEM_H
#define MEM_H

/** \addtogroup Memory
  * \brief Memory management functions
  * \ingroup Common
  * @{
  */
#include <ac/stddef.h>
#include <main.h>

/** \brief allocate memory
 *
 *  mem_alloc uses malloc to allocate the required sized chunk of
 *  memory.  If any error is returned from malloc() a fatal
 *  diagnostic is issued.
 *
 *  @param The size of the memory to allocate.
 *
 *  @warning It is the responsibility of the caller to ensure that the
 *  space is freed when finished with, by a call to mem_free().
 */

void *mem_alloc(size_t);


/** \brief allocate and clear memory
 *
 *  mem_alloc_clear uses malloc to allocate the required sized chunk
 *  of memory.  If any error is returned from malloc() an fatal
 *  diagnostic is issued.  The memory is zeroed befor it is returned.
 *
 *  @param n - The size of the memory to allocate
 *
 *  @warning It is the responsibility of the caller to ensure that the space is
 *	freed when finished with, by a call to free().
 */

void *mem_alloc_clear(size_t);


void *mem_change_size(void *, size_t);
void mem_free(void *);
char *mem_copy_string(const char *);

/** @} */
#endif /* MEM_H */
