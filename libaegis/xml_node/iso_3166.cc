//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/xml_node/iso_3166.h>


xml_node_iso_3166::~xml_node_iso_3166()
{
}


xml_node_iso_3166::xml_node_iso_3166()
{
}


bool
xml_node_iso_3166::need_setup(void)
{
    if (codes.empty())
    {
        assign("au", "aus");
        assign("aus", "au");
        return true;
    }
    return false;
}


bool
xml_node_iso_3166::is_a_valid_code(const nstring &code_name)
    const
{
    codes_t::const_iterator it = codes.find(code_name.downcase());
    return (it != codes.end());
}


void
xml_node_iso_3166::assign(const nstring &name, const nstring &value)
{
    codes.insert(codes_t::value_type(name.downcase(), value));
}


void
xml_node_iso_3166::element_begin(const nstring &)
{
    alpha_2_code.clear();
    alpha_3_code.clear();
}


void
xml_node_iso_3166::attribute(const nstring &name, const nstring &value)
{
    if (name == "alpha_2_code")
        alpha_2_code = value;
    else if (name == "alpha_3_code")
        alpha_3_code = value;
}


void
xml_node_iso_3166::element_end(const nstring &)
{
    if (alpha_2_code.empty())
    {
        if (alpha_3_code.empty())
        {
            // do nothing with nothing
        }
        else
        {
            assign(alpha_3_code, alpha_3_code);
        }
    }
    else
    {
        if (alpha_3_code.empty())
        {
            assign(alpha_2_code, alpha_2_code);
        }
        else
        {
            assign(alpha_2_code, alpha_3_code);
            assign(alpha_3_code, alpha_2_code);
        }
    }
}


// vim: set ts=8 sw=4 et :
