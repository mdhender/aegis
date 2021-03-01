//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
//	Copyright (C) 2006 Walter Franzini;
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

#ifndef LIBAEGIS_ATTRIBUTE_H
#define LIBAEGIS_ATTRIBUTE_H

#include <libaegis/common.h>

/**
  * The attributes_list_find function is used to find a specific
  * attribute in an attribute list.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  */
attributes_ty *attributes_list_find(attributes_list_ty *alp, const char *name);

/**
  * The attributes_list_find_boolean function is used to find a specific
  * attribute in an attribute list.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  * \param default_value
  *     The value to use of the attribute is not present.
  * \returns
  *     bool; the value of the attribute as a boolean (it understands
  *     "yes", "no", "true" and "false"), or the default value if not
  *     present or indecipherable.
  */
bool attributes_list_find_boolean(attributes_list_ty *alp, const char *name,
    bool default_value = false);

/**
  * The attributes_list_find_read function is used to find a specific
  * attribute in an attribute list.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  * \param default_value
  *     The value to use of the attribute is not present.
  * \returns
  *     double; the value of the attribute as a floating point(real)
  *     number, or the default value if not present or indecipherable.
  */
double attributes_list_find_real(attributes_list_ty *alp, const char *name,
    double default_value = 0);

/**
  * The attributes_list_find_integer function is used to find a specific
  * attribute in an attribute list.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  * \param default_value
  *     The value to use of the attribute is not present.
  * \returns
  *     long; the value of the attribute as an integer number, or the
  *     default value if not present or indecipherable.
  */
long attributes_list_find_integer(attributes_list_ty *alp, const char *name,
    long default_value = 0);

/**
  * The attributes_list_extract function is used to find a specific
  * attribute in an attribute list, and remove it.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  * \returns
  *     NULL if the attribute is not present, otherwise it returns a
  *     pointer to the attribute removed from the list.
  */
attributes_ty *attributes_list_extract(attributes_list_ty *alp,
    const char *name);

/**
  * The attributes_list_remove function is used to find a specific
  * attribute in an attribute list, and remove it.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to search for; not case sensitive.
  */
void attributes_list_remove(attributes_list_ty *alp, const char *name);

/**
  * The attributes_list_insert function is used to find a specific
  * attribute in an attribute list, and replace its value with the one
  * given.  If there is not attribute with the given name, this name and
  * value will be appended.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to modify; not case sensitive.
  * \param value
  *     The value of the attribute to be set.
  */
void attributes_list_insert(attributes_list_ty *alp, const char *name,
    const char *value);

/**
  * The attributes_list_append function is used to add a specific
  * attribute name and value pair to the end of an attribute list.  It
  * is possible to get duplicate attributes with the same name by using
  * this function; use attributes_list_insert function if you don't want
  * duplicates.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to append; not case sensitive.
  * \param value
  *     The value of the attribute to be set.
  */
void attributes_list_append(attributes_list_ty *alp, const char *name,
    const char *value);

/**
  * The attributes_list_append_unique function is used to add a specific
  * attribute name and value pair to the end of an attribute list
  * <b>if</b> that name and value pair is not already present.  It is
  * possible to get duplicate attributes with the same name by using
  * this function; use the attributes_list_insert function if you don't
  * want duplicates.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to append; not case sensitive.
  * \param value
  *     The value of the attribute to be set.
  */
void attributes_list_append_unique(attributes_list_ty *alp, const char *name,
    string_ty *value);

/**
  * The attributes_list_append_unique function is used to add a specific
  * attribute name and value pair to the end of an attribute list
  * <b>if</b> that name and value pair is not already present.  It is
  * possible to get duplicate attributes with the same name by using
  * this function; use the attributes_list_insert function if you don't
  * want duplicates.
  *
  * \param alp
  *     The attribute list to be searched.
  * \param name
  *     The name of the attribute to append; not case sensitive.
  * \param value
  *     The value of the attribute to be set.
  */
void attributes_list_append_unique(attributes_list_ty *alp, const char *name,
    const char *value);

#endif // LIBAEGIS_ATTRIBUTE_H
