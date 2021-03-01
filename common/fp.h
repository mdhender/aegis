//
//	cook - file construction tool
//	Copyright (C) 1994, 2003-2006, 2008 Peter Miller.
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

#ifndef COMMON_FP_H
#define COMMON_FP_H

#include <common/ac/stddef.h>

/** \addtogroup Fingerprint
  * \brief Fingerprint types and functions
  * \ingroup Common
  * @{
  */
struct fingerprint_methods_ty; // forward

#define FINGERPRINT_BASE_CLASS \
	struct fingerprint_methods_ty *method;

struct fingerprint_ty
{
	FINGERPRINT_BASE_CLASS
};

struct fingerprint_methods_ty
{
    long	    size;
    const char      *name;
    void	    (*constructor)(fingerprint_ty *);
    void	    (*destructor)(fingerprint_ty *);
    void	    (*addn)(fingerprint_ty *, unsigned char *, size_t);
    int		    (*hash)(fingerprint_ty *, unsigned char *);
    void	    (*sum)(fingerprint_ty *, char *, size_t);
};

fingerprint_ty *fingerprint_new(fingerprint_methods_ty *);
void fingerprint_delete(fingerprint_ty *);
void fingerprint_add(fingerprint_ty *, int);
int fingerprint_file_hash(fingerprint_ty *, const char *, unsigned char *);
int fingerprint_file_sum(fingerprint_ty *, const char *, char *, size_t);

#define fingerprint_addn(p, s, n) \
	(p)->method->addn((p), (s), (n))
#define fingerprint_hash(p, s) \
	(p)->method->hash((p), (s))
#define fingerprint_sum(p, s, len) \
	(p)->method->sum((p), (s), (len))

/** @} */
#endif // COMMON_FP_H
