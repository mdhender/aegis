//
//      aegis - project change supervisor
//      Copyright (C) 1994, 2002-2008, 2012 Peter Miller.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_SYMTAB_H
#define COMMON_SYMTAB_H

#include <common/mem.h>
#include <common/nstring.h>

class string_list_ty; // forward
class nstring_list; // forward

/** @addtogroup Symtab
  * @brief Symbols table interface
  * @ingroup Common
  * @{
  */

/**
  * The symtab_ty class is used to represent a symbol table.  All data
  * is referenced through void pointers.  You may wish to use the
  * template wrapper for type safety.
  */
class symtab_ty
{
public:
    /**
      * The destructor.
      * @note it isn't virtual, thou shalt not derive from this class.
      */
    ~symtab_ty();

    /**
      * The constructor.
      *
      * @param suggested_size
      *     You are able to suggest how many rows will be in the table.
      *     It is better to under estimate than overestimate and waste
      *     memory.  Optimal resizing happens automagically.
      */
    symtab_ty(int suggested_size = 5);

    /**
      * The size method may be used to determine how many rows there are
      * in the symbol table.
      */
    size_t size() const { return hash_load; }

    /**
      * The empty method may be used to determine if there symbol table
      * is empty (i.e. there are no rows).
      */
    bool empty() const { return (hash_load == 0); }

    /**
      * The clear method may be used to discard all rows of the symbol
      * table.  It is not an error if the symbol table is already empty.
      *
      * @note
      *     This method has O(n) execution time.
      */
    void clear(void);

    /**
      * The query method may be used to search for a variable.
      *
      * @param key
      *     The row name to search for.
      *
      * @returns
      *     If the variable has been defined, this method returns the
      *     pointer value assigned.  If the variable has not been
      *     defined, it returns the NULL pointer.
      *
      * @note
      *     This method has O(1) execution time.
      *     This method will eventually be DEPRECATED
      */
    void *query(string_ty *key) const;

    /**
      * The query method may be used to search for a variable.
      *
      * @param key
      *     The row name to search for.
      *
      * @returns
      *     If the variable has been defined, this method returns the
      *     pointer value assigned.  If the variable has not been
      *     defined, it returns the NULL pointer.
      *
      * @note
      *     This method has O(1) execution time.
      */
    void *query(const nstring &key) const;

    /**
      * The query method may be used to search for a variable.
      *
      * @param keys
      *     The row names to search for.  The first found will be returned.
      *
      * @returns
      *     If the variable has been defined, this method returns the
      *     pointer value assigned.  If the variable has not been
      *     defined, it returns the NULL pointer.
      */
    void *query(const nstring_list &keys) const;

    /**
      * The query_fuzzy method may be used to search for a variable.
      *
      * @param key
      *     The row name to search for.
      *
      * @returns
      *     The NULL pointer if there is no row of that name and no row
      *     with a similar name, otherwise returns a pointer to the most
      *     similar name.
      *
      * @note
      *     This method has O(n) execution time.
      *     This method will eventually be DEPRECATED
      */
    string_ty *query_fuzzy(string_ty *key) const;

    /**
      * The query_fuzzy method may be used to search for a variable.
      *
      * @param key
      *     The row name to search for.
      *
      * @returns
      *     The empty string if there is no row of that name and no row with
      *     a similar name, otherwise returns the most similar name.
      *
      * @note
      *     This method has O(n) execution time.
      */
    nstring query_fuzzy(const nstring &key) const;

    /**
      * The assign method is used to assign a value to a given variable.
      *
      * @param key
      *     They key (usually a variable name or simialar).
      * @param value
      *     The value to be assigned to that name.
      *
      * @note
      *     The key is copied, the value pointed to is not.
      * @note
      *     If there is already a key of that name, the old data will be
      *     discarded, via the reap function, if one has been supplied.
      * @note
      *     This method has O(1) execution time.
      *     This method will eventually be DEPRECATED
      */
    void assign(string_ty *key, void *value);

    /**
      * The assign method is used to assign a value to a given variable.
      *
      * @param key
      *     They key (usually a variable name or simialar).
      * @param value
      *     The value to be assigned to that name.
      *
      * @note
      *     The key is copied, the value pointed to is not.
      * @note
      *     If there is already a key of that name, the old data will be
      *     discarded, via the reap function, if one has been supplied.
      * @note
      *     This method has O(1) execution time.
      */
    void assign(const nstring &key, void *value);

    /**
      * The assign_push function is used to assign a value to a given
      * variable.  Any previous value will be obscured until this one is
      * removed with the remove method.
      *
      * @param key
      *     They key (usually a variable name or simialar).
      * @param value
      *     The value to be assigned to that name.
      *
      * @note
      *     The key is copied, the value pointed to is not.
      * @note
      *     This method has O(1) execution time.
      *     This method will eventually be DEPRECATED
      */
    void assign_push(string_ty *key, void *value);

    /**
      * The assign_push function is used to assign a value to a given
      * variable.  Any previous value will be obscured until this one is
      * removed with the remove method.
      *
      * @param key
      *     They key (usually a variable name or simialar).
      * @param value
      *     The value to be assigned to that name.
      *
      * @note
      *     The key is copied, the value pointed to is not.
      * @note
      *     This method has O(1) execution time.
      */
    void assign_push(const nstring &key, void *value);

    /**
      * The remove method is used to remove a variable from the symbol table.
      *
      * @param key
      *     The name of the row to be removed.
      *
      * @note
      *    The name is freed, the data is reaped.
      *    (By default, reap does nothing.)
      * @note
      *     This method has O(1) execution time.
      *     This method will eventually be DEPRECATED
      */
    void remove(string_ty *key);

    /**
      * The remove method is used to remove a variable from the symbol table.
      *
      * @param key
      *     The name of the row to be removed.
      *
      * @note
      *    The name is freed, the data is reaped.
      *    (By default, reap does nothing.)
      * @note
      *     This method has O(1) execution time.
      */
    void remove(const nstring &key);

    /**
      * The dump method is used to dump the contents of the symbol
      * table.
      *
      * @param caption
      *     The caption will be used to indicate why the symbol
      *     table was dumped.
      *
      * @note
      *    This function is only available when symbol DEBUG is defined.
      * @note
      *     This method has O(n) execution time.
      */
    void dump(const char *caption) const;

    /**
      * The keys method may be used to extract the list of row names
      * from the symbol table.
      *
      * @param result
      *     Where to put the row names.  It is cleared before any row
      *     names are placed in it.  It is not sorted.
      *
      * @note
      *     If you have used assign_push method, it is possible to have
      *     duplicates in they list of keys.
      * @note
      *     This method has O(n) execution time.
      *     This method will eventually be DEPRECATED
      */
    void keys(string_list_ty *result) const;

    /**
      * The keys method may be used to extract the list of row names
      * from the symbol table.
      *
      * @param result
      *     Where to put the row names.  It is cleared before any row
      *     names are placed in it.  It is not sorted.
      *
      * @note
      *     If you have used assign_push method, it is possible to have
      *     duplicates in they list of keys.
      * @note
      *     This method has O(n) execution time.
      */
    void keys(nstring_list &result) const;

    typedef void (*callback_t)(const symtab_ty *stp, const nstring &key,
        void *data, void *arg);

    /**
      * The walk method is used to invoke a func tion for every row of
      * the symbol table.
      *
      * @param func
      *     A pointer to the function to be called.
      * @param arg
      *     An extra argument passed to the function.
      *
      * @note
      *     This method has O(n) execution time.
      */
    void walk(callback_t func, void *arg) const;

    typedef void (*reaper_t)(void *);

    /**
      * The set_reap method is used to set the reaping function to be
      * used on the data of row tables when they are remove()ed or
      * assign()ed over.
      */
    void set_reap(reaper_t func) { reap = func; }

    /**
      * The valid method determines whether the symbol table's internal
      * values are self consistent.  Usually only used for debugging.
      */
    bool valid() const;

private:
    /**
      * The split method is used to double the number of buckets in the
      * symbol table, which results in halving the load.  The symbols
      * are then redistributed into the new buckets.
      *
      * @note
      *    It is only sensable to do this when the symbol table load
      *    exceeds some reasonable threshold.  A threshold of 80% is
      *    used.
      * @note
      *     The probablity of another split thus halves every time this
      *     method is called, resulting in overall O(1) behaviour
      *     because (sigma(2 ** -n) == 1).
      */
    void split(void);

    /**
      * The grim reaper.  Default to NULL, i.e. nothing is done.
      */
    reaper_t reap;

    struct row_t
    {
        row_t() : data(0), overflow(0) { }
        nstring key;
        void *data;
        row_t *overflow;
    };

    /**
      * The hash_table instance variable is used to remember the base
      * address of the array of buckets.  On average, there will only be
      * 0.5 to 0.8 rows per bucket.
      */
    row_t **hash_table;

    /**
      * The hash_modulus instance variable is used to remember the size
      * of the array of buckets.  It is always a power of two.
      */
    str_hash_ty hash_modulus;

    /**
      * The hash_mask instance variable is used to remember the bit mask
      * used to place rows into buckets.  Because hash_modulus is always
      * a power of two, this mask will always be one less than the hash
      * modulus; we cache it for efficiency.
      */
    str_hash_ty hash_mask;

    /**
      * The hash_load instance variable is used to remember how many
      * rows are present in the table.
      */
    str_hash_ty hash_load;

    /**
      * The copy constructor.  Do not use.
      */
    symtab_ty(const symtab_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    symtab_ty &operator=(const symtab_ty &);

    friend class symtab_iterator;
};

symtab_ty *symtab_alloc(int n) DEPRECATED;
void symtab_free(symtab_ty *stp) DEPRECATED;

void *symtab_query(const symtab_ty *stp, string_ty *key) DEPRECATED;
string_ty *symtab_query_fuzzy(const symtab_ty *stp, string_ty *key) DEPRECATED;
void symtab_assign(symtab_ty *stp, string_ty *key, void *value) DEPRECATED;
void symtab_assign_push(symtab_ty *stp, string_ty *key, void *value) DEPRECATED;
void symtab_delete(symtab_ty *stp, string_ty *key) DEPRECATED;
void symtab_dump(const symtab_ty *stp, const char *caption) DEPRECATED;
void symtab_walk(const symtab_ty *stp, symtab_ty::callback_t func, void *arg)
    DEPRECATED;
void symtab_keys(const symtab_ty *stp, string_list_ty *result) DEPRECATED;

/** @} */

#endif // COMMON_SYMTAB_H
// vim: set ts=8 sw=4 et :
