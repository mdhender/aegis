//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the symtab_template class
//

#ifndef COMMON_SYMTAB_TEMPLATE_H
#define COMMON_SYMTAB_TEMPLATE_H

#pragma interface "symtab"

#include <nstring.h>
#include <nstring/list.h>
#include <symtab.h>

/** \addtogroup Symtab
  * \brief Symbols table interface
  * \ingroup Common
  * @{
  */


/**
  * The symtab template class is used to provide a type-safe interface
  * to the generic symtab_* API.
  *
  * This template is intended as a <i>very</i> thin veneer over the
  * underlying functionality.  For this reason all of the methods are
  * inline.  If you add more methods, make sure they are also inline.
  */
template <class value_type_t>
class symtab
{
public:
    /**
      * The destructor.
      * It is not virtual, do not derive from this class.
      */
    ~symtab()
    {
	if (stp)
	{
	    delete stp;
	    stp = 0;
	}
    }

    /**
      * The default constructor.
      */
    symtab() :
	stp(0)
    {
    }

    /**
      * The query method is used to locate the given key in the symbol table.
      *
      * @param key
      *     The symbol table entry to look for.
      */
    value_type_t *
    query(string_ty *key)
	const
    {
	if (!stp)
	    return 0;
	return (value_type_t *)stp->query(key);
    }

    /**
      * The query method is used to locate the given key in the symbol table.
      *
      * @param key
      *     The symbol table entry to look for.
      */
    value_type_t *
    query(const nstring &key)
	const
    {
	if (!stp)
	    return 0;
	return (value_type_t *)stp->query(key.get_ref());
    }

    /**
      * The assign method is used to associate a value with a key.
      *
      * @param key
      *     The symbol table entry to set.
      * @param value
      *     The value to assign.  Note that it is always a pointer.
      *     If you have called the set_reaper method, it will have
      *     operator delete called on it (non array) when the symbol
      *     table destructor is run.
      */
    void
    assign(string_ty *key, value_type_t *value)
    {
	if (!stp)
	    stp = new symtab_ty(5);
	stp->assign(key, (void *)value);
    }

    /**
      * The assign mentod is used to associate a value with a key.
      *
      * @param key
      *     The symbol table entry to set.
      * @param value
      *     The value to assign.  Note that it is always a pointer.
      *     If you have called the set_reaper method, it will have
      *     operator delete called on it (non array) when the symbol
      *     table destructor is run.
      */
    void
    assign(const nstring &key, value_type_t *value)
    {
	if (!stp)
	    stp = new symtab_ty(5);
	stp->assign(key.get_ref(), (void *)value);
    }

    /**
      * The assign method is used to associate a value with a key.
      *
      * @param key
      *     The symbol table entry to set.
      * @param value
      *     The value to assign.  A copy will be made in dynamic memory.
      *     You need to call set_reaper to ensure there is no memory
      *     leak when the symbol table destructor is run.
      */
    void
    assign(const nstring &key, const value_type_t &value)
    {
	// I hope they called set_reaper already.
	assign(key, new value_type_t(value));
    }

    /**
      * The remove method is used to remove a value (and its key) from a
      * symbol table. It is not an error if it has already been removed.
      *
      * @param key
      *     The symbol table entry to look for.
      * @note
      *     If set_reaper has been called, the operator delete will be
      *     called for the value.
      */
    void
    remove(const nstring &key)
    {
	if (stp)
	    stp->remove(key.get_ref());
    }

    /**
      * The dump method is used during debugging to print the contents
      * of a symbol table.
      *
      * @param caption
      *     The heading to print before the contents.
      */
    void
    dump(const char *caption)
	const
    {
	if (stp)
	    stp->dump(caption);
    }

    /**
      * The set_reaper method is used to set the reper function of the
      * inner symtab_ty.  This not done by default, because not all
      * symbol tables need their contents deleted when they are deleted.
      */
    void
    set_reaper()
    {
	if (!stp)
	    stp = new symtab_ty(5);
	stp->set_reap(reaper);
    }

    /**
      * The empty method may be used to determine if there symbol table
      * is empty (i.e. there are no rows).
      */
    bool
    empty()
	const
    {
	return (!stp || stp->empty());
    }

    /**
      * The size method may be used to determine how many rows there are
      * in the symbol table.
      */
    size_t
    size()
	const
    {
	return (stp ? stp->size() : 0);
    }

    /**
      * The keys method may be used to extract the list of row names
      * from the symbol table.
      *
      * \param result
      *     Where to put the row names.  It is cleared before any row
      *     names are placed in it.  It is not sorted.
      *
      * \note
      *     If you have used assign_push method, it is possible to have
      *     duplicates in the list of keys.
      * \note
      *     This method has O(n) execution time.
      */
    void
    keys(nstring_list &result)
    {
	if (stp)
	    stp->keys(result);
	else
	    result.clear();
    }

private:
    /**
      * The stp instance variable is used to remember the location of
      * the dynamically allocated symbol table.
      */
    symtab_ty *stp;

    /**
      * The reaper class methos is used to delete symbol table contents
      * when assigned over, and when the symbol table destructor is run.
      */
    static void reaper(void *p) { delete (value_type_t *)p; }

    /**
      * The copy constructor.  Do not use.
      */
    symtab(const symtab &);

    /**
      * The assignment operator.  Do not use.
      */
    symtab &operator=(const symtab &);
};

/** @} */

#endif // COMMON_SYMTAB_TEMPLATE_H
