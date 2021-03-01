/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1996, 1997, 2003, 2004 Peter Miller;
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
 * MANIFEST: interface definition for common/str_list.c
 */

#ifndef COMMON_STR_LIST_H
#define COMMON_STR_LIST_H

#include <str.h>

/** \addtogroup String_List
  * \brief String Lists
  * \ingroup String
  * @{
  */

/**
  * The string_list_ty type is used to represent a list of string
  * (string_ty *) values.
  */
typedef struct string_list_ty string_list_ty;
struct string_list_ty
{
	size_t		nstrings;
	size_t		nstrings_max;
	string_ty	**string;
};

/**
  * The string_list_member function is used to test for the existence
  * of a string in a string list.  (This is an O(n) search.)
  */
int string_list_member(string_list_ty *, string_ty *);

/**
  * The wl2str function is used to convert a list of strings into a
  * single string, separated by the given separator.
  *
  * @param slp
  *	The string list to be joined.
  * @param start
  *	The index of the first string in the list to be joined (counting
  *	from zero).
  * @param finish
  *	The index of the last string in the list to be joined (counting
  *	from one, or one past the end, counting from zero).
  */
string_ty *wl2str(string_list_ty *slp, int start, int finish, const char *sep);

/**
  * The str2wl function is used to break a string up into several strings
  * at a given separatopr character.
  *
  * @param slp
  *	The place to put the resulting list of strings.  They are in
  *	the same order as they appeared in the input string (arg).
  * @param arg
  *	The input string to be split into component strings.
  * @param sep
  *	This is the set of separator characters.  Supostrings will be
  *	extracted if *any* of the characters in this strings are seen
  *	(like $IFS in the shell).  If NULL, white space will be used.
  * @param white
  *	If true (non-zero) leading ans trailing white space will be
  *	removed from the result strings.  If false (zero) white space
  *	will be retained.
  */
void str2wl(string_list_ty *slp, string_ty *arg, const char *sep, int white);

/**
  * The string_list_prepend function is used to add another string to
  * the start of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the front of lhs.
  */
void string_list_prepend(string_list_ty *lhs, string_ty *rhs);

/**
  * The string_list_prepend_list function is used to add all the strings
  * of another string list to the start of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the front of lhs.
  */
void string_list_prepend_list(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_append function is used to add another string to
  * the end of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the end of lhs.
  */
void string_list_append(string_list_ty *lhs, string_ty *rhs);

/**
  * The string_list_append_list function is used to add all the strings
  * of another string list to the end of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the end of lhs.
  */
void string_list_append_list(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_append_unique function is used to add another string
  * to the end of a string list, provided it is not already present.
  * This is an O(n) operation.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the end of lhs.
  */
void string_list_append_unique(string_list_ty *lhs, string_ty *rhs);

/**
  * The string_list_append_list_unique function is used to add all the
  * strings of another string list to the end of a string list, provided
  * they are not already present.  This is an O(n**2) operation.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the end of lhs.
  */
void string_list_append_list_unique(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_copy function is sued to replace the contents of one
  * string list with another.  This is more like a copy constructor than
  * an assignment operator.  The previous contents will be discarded
  * NOT free()ed.
  */
void string_list_copy(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_remove function is used to remove a speific valued
  * string from a string list.  If there are no occurrences, it is not
  * an error.  If there is more than one occurrece, only the first will
  * be removed.  This is an O(n) operation.
  */
void string_list_remove(string_list_ty *, string_ty *);

/**
  * The string_list_remove_list function is used to remove all instances
  * of one string list (rhs) from another string list (lhs).
  *
  * @param lhs
  *	The string list to have element removed.
  * @param rhs
  *	The list of values to be removed from lhs.
  */
void string_list_remove_list(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_destructor function is used to release all resources
  * held by a string list.  This function shall be called when you are
  * finished with a string list.
  */
void string_list_destructor(string_list_ty *);

/**
  * The string_list_delete function is used to release all resources
  * held by a dynamically allocated string list (created by the
  * string_list_new() function).  This function shall be called when
  * you are finished with a dynamically allocated string list.
  */
void string_list_delete(string_list_ty *);

/**
  * The string_list_constructor function is used to prepare a string
  * list for use.  (Note: strings lists which are global variables do
  * NOT need this function called before use).
  */
void string_list_constructor(string_list_ty *);

/**
  * The string_list_new function is used to allocate a string list
  * in dynamic memory and prepare it for use.  You shall call the
  * string_list_delete() function when you are finished with it.
  */
string_list_ty *string_list_new(void);

/**
  * The string_list_equal function is used to determine if two string
  * lists contain the same strings.  The *order* of the strings is not
  * considered. This is an O(n**2) operation.
  *
  * @return
  *	Returns true (non-zero) if the two sets of strings are the same,
  *	false (zero) if they are not.
  */
int string_list_equal(string_list_ty *, string_list_ty *);

/**
  * The string_list_subset function is used to determine if lhs is an
  * (improper) subset of rhs.
  *
  * @return
  *	Returns true (non-zero) if the two sets of strings are the same,
  *	false (zero) if they are not.
  */
int string_list_subset(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_sort function is used to sort a list of strings.
  * The comparison function is strcmp(3).  This is an O(n log n)
  * operation.
  */
void string_list_sort(string_list_ty *);

/**
  * The string_list_sort_version function is used to sort a list of
  * strings.  The comparison function is strverscmp(3).  This is an O(n
  * log n) operation.
  */
void string_list_sort_version(string_list_ty *);

/**
  * The string_list_quote_shell function is used to shell quote each
  * string in a list of strings.  The quoting is done by using the
  * str_quote_shell() function.
  *
  * @param lhs
  *	The place to put this list of quoted strings.
  * @param rhs
  *	The unquoted input strings.
  */
void string_list_quote_shell(string_list_ty *lhs, string_list_ty *rhs);

/**
  * The string_list_validate function is used to validate the internal
  * structure of a string list.  It is used for debugging, and shall
  * not be used when the DEBUG symbol is not defined.
  */
int string_list_validate(string_list_ty *);

/** @} */
#endif /* COMMON_STR_LIST_H */
