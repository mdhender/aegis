//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1999, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_MEM_H
#define COMMON_MEM_H

/** \addtogroup Memory
  * \brief Memory management functions
  * \ingroup Common
  * @{
  */
#include <common/ac/stddef.h>
#include <common/ac/new.h>
#include <common/main.h>

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

#ifdef DMALLOC
void *dmem_alloc(const char *file, int line, size_t nbytes);
#define mem_alloc(nbytes) dmem_alloc(__FILE__, __LINE__, (nbytes))
#endif

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
#ifdef DMALLOC
void *dmem_alloc_clear(const char *file, int line, size_t nbytes);
#define mem_alloc_clear(nbytes) dmem_alloc_clear(__FILE__, __LINE__, (nbytes))
#endif


void *mem_change_size(void *, size_t);
#ifdef DMALLOC
void *dmem_change_size(const char *file, int line, void *p,
    size_t nbytes);
#define mem_change_size(p, nbytes) \
   dmem_change_size(__FILE__, __LINE__, (p), (nbytes))
#endif

void mem_free(void *);
#ifdef DMALLOC
void dmem_free(const char *file, int line, void *p);
#define mem_free(p) dmem_free(__FILE__, __LINE__, (p))
#endif


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
#ifdef DMALLOC
char *dmem_copy_string(const char *file, int line, const char *p);
#define mem_copy_string(p) dmem_copy_string(__FILE__, __LINE__, (p))
#endif


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
#ifdef DMALLOC
char *dmem_copy_string(const char *file, int line,
    const char *p, size_t len);
#undef mem_copy_string
#define mem_copy_string(p, ...) \
   dmem_copy_string(__FILE__, __LINE__, (p), ## __VA_ARGS__)
#endif

#if HAVE_HEADER_NEW || HAVE_NEW_H
#define THROW_BAD_ALLOC throw(std::bad_alloc)
#else
#define THROW_BAD_ALLOC
#endif

#ifndef DMALLOC
void *operator new(size_t nbytes) THROW_BAD_ALLOC;
#else
void *operator new(size_t nbytes, const char *file, int line)
    THROW_BAD_ALLOC;
#endif

#ifndef DMALLOC
void *operator new[](size_t nbytes) THROW_BAD_ALLOC;
#else
void *operator new[](size_t nbytes, const char *file, int line)
    THROW_BAD_ALLOC;
#endif

#ifndef DMALLOC
void operator delete(void *ptr) throw();
#else
void operator delete(void *ptr, const char *file, int line) throw();
#endif

#ifndef DMALLOC
void operator delete[](void *ptr) throw();
#else
void operator delete[](void *ptr, const char *file, int line) throw();
#endif

#if HAVE_HEADER_NEW || HAVE_NEW_H

#ifndef DMALLOC
void *operator new(size_t nbytes, std::nothrow_t &) throw();
#else
void *operator new(size_t nbytes, std::nothrow_t &, const char *file,
    int line) throw();
#endif

#ifndef DMALLOC
void *operator new[](size_t nbytes, std::nothrow_t &) throw();
#else
void *operator new[](size_t nbytes, std::nothrow_t &, const char *file,
    int line) throw();
#endif

#ifndef DMALLOC
void operator delete(void *ptr, std::nothrow_t const &) throw();
#else
void operator delete(void *ptr, std::nothrow_t const &, const char *file,
    int line) throw();
#endif

#ifndef DMALLOC
void operator delete[](void *ptr, std::nothrow_t const &) throw();
#else
void operator delete[](void *ptr, std::nothrow_t const &, const char *file,
    int line) throw();
#endif

#ifdef DMALLOC
#define new new(__FILE__, __LINE__)
//#define delete delete(__FILE__, __LINE__)
#endif

#endif

/** @} */
#endif // COMMON_MEM_H
