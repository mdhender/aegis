//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2012 Peter Miller
//      Copyright (C) 2006 Walter Franzini
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

#include <common/ac/assert.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <common/boolean.h>

#include <libaegis/attribute.h>


attributes_ty *
attributes_list_find(attributes_list_ty *alp, const char *name)
{
    if (!alp)
        return 0;
    for (size_t j = 0; j < alp->length; ++j)
    {
        attributes_ty *ap = alp->list[j];
        assert(ap);
        assert(ap->name);
        assert(ap->value);
        if (ap->name && ap->value && 0 == strcasecmp(ap->name->str_text, name))
            return ap;
    }
    return 0;
}


bool
attributes_list_find_boolean(attributes_list_ty *alp, const char *name,
    bool default_value)
{
    attributes_ty *ap = attributes_list_find(alp, name);
    if (!ap || !ap->value)
        return default_value;
    return string_to_bool(nstring(ap->value), default_value);
}


double
attributes_list_find_real(attributes_list_ty *alp, const char *name,
    double default_value)
{
    attributes_ty *ap = attributes_list_find(alp, name);
    if (!ap || !ap->value)
        return default_value;
    char *ep = 0;
    double result = strtod(ap->value->str_text, &ep);
    if (ep == ap->value->str_text || *ep)
        return default_value;
    return result;
}


long
attributes_list_find_integer(attributes_list_ty *alp, const char *name,
    long default_value)
{
    attributes_ty *ap = attributes_list_find(alp, name);
    if (!ap || !ap->value)
        return default_value;
    char *ep = 0;
    long result = strtol(ap->value->str_text, &ep, 0);
    if (ep == ap->value->str_text || *ep)
        return default_value;
    return result;
}


void
attributes_list_remove(attributes_list_ty *alp, const char *name)
{
    attributes_ty *ap = attributes_list_extract(alp, name);
    if (ap)
        attributes_type.free(ap);
}


attributes_ty *
attributes_list_extract(attributes_list_ty *alp, const char *name)
{
    if (!alp)
        return 0;
    for (size_t j = 0; j < alp->length; ++j)
    {
        attributes_ty *ap = alp->list[j];
        assert(ap);
        assert(ap->name);
        if (ap->name && 0 == strcasecmp(ap->name->str_text, name))
        {
            for (size_t k = j + 1; k < alp->length; ++k)
                alp->list[k - 1] = alp->list[k];
            alp->length--;
            return ap;
        }
    }
    return 0;
}


void
attributes_list_insert(attributes_list_ty *alp, const char *name,
    const char *value)
{
    attributes_ty *ap = attributes_list_find(alp, name);
    if (ap)
    {
        assert(ap->value);
        if (ap->value)
            str_free(ap->value);
        ap->value = str_from_c(value);
        return;
    }
    attributes_list_append(alp, name, value);
}


void
attributes_list_append(attributes_list_ty *alp, const char *name,
    const char *value)
{
    assert(alp);
    if (!alp)
        return;
    meta_type *type;
    attributes_ty **app =
        (attributes_ty **)attributes_list_type.list_parse(alp, &type);
    assert(type == &attributes_type);
    assert(app);
    attributes_ty *ap = (attributes_ty *)attributes_type.alloc();
    ap->name = str_from_c(name);
    ap->value = str_from_c(value);
    *app = ap;
}


void
attributes_list_append_unique(attributes_list_ty *alp, const char *name,
    string_ty *value)
{
    assert(alp);
    if (!alp)
        return;

    //
    // If this exact name and value is already present,
    // return without doing anything.
    //
    for (size_t j = 0; j < alp->length; ++j)
    {
        attributes_ty *ap = alp->list[j];
        assert(ap);
        assert(ap->name);
        assert(ap->value);
        if
        (
            0 == strcasecmp(ap->name->str_text, name)
        &&
            str_equal(ap->value, value)
        )
        {
            return;
        }
    }

    //
    // Now that we know it is a unique pair,
    // append it to the end of the list.
    //
    attributes_list_append(alp, name, value->str_text);
}

void
attributes_list_append_unique(attributes_list_ty *alp, const char *name,
    const char *value)
{
    assert(alp);
    if (!alp)
        return;

    //
    // If this exact name and value is already present,
    // return without doing anything.
    //
    for (size_t j = 0; j < alp->length; ++j)
    {
        attributes_ty *ap = alp->list[j];
        assert(ap);
        assert(ap->name);
        assert(ap->value);
        if
        (
            0 == strcasecmp(ap->name->str_text, name)
        &&
            0 == strcmp(ap->value->str_text, value)
        )
        {
            return;
        }
    }

    //
    // Now that we know it is a unique pair,
    // append it to the end of the list.
    //
    attributes_list_append(alp, name, value);
}


// vim: set ts=8 sw=4 et :
