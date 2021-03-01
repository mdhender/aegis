//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2004-2008 Peter Miller
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

#ifndef COMMON_SYMTAB_ITER_H
#define COMMON_SYMTAB_ITER_H

#include <common/symtab.h>

/**
  * The symtab_iterator class is used to iterate across all rows in a
  * symbols table.
  */
class symtab_iterator
{
public:
    ~symtab_iterator();
    symtab_iterator(const symtab_ty *);

    void reset(void);
    bool next(string_ty **key, void **data);
    bool next(nstring &key, void *&data);

private:
    //
    // These fields are for internal use only, and are not to be
    // accessed or manipulated by clents of this interface.
    //
    const symtab_ty *stp;
    symtab_ty::row_t *rp;
    str_hash_ty pos;

    symtab_iterator();
    symtab_iterator(const symtab_iterator &);
    symtab_iterator &operator=(const symtab_iterator &);
};

inline symtab_iterator *
symtab_iterator_new(symtab_ty *arg)
{
    return new symtab_iterator(arg);
}

inline void
symtab_iterator_delete(symtab_iterator *stip)
{
    delete stip;
}

inline void
symtab_iterator_reset(symtab_iterator *stip)
{
    stip->reset();
}

inline int
symtab_iterator_next(symtab_iterator *stip, string_ty **key_p, void **data_p)
{
    return stip->next(key_p, data_p);
}

#endif // COMMON_SYMTAB_ITER_H
