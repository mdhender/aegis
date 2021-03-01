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

#include <common/trace.h>
#include <libaegis/xml_node/iso_639_3.h>


xml_node_iso_639_3::~xml_node_iso_639_3()
{
}


xml_node_iso_639_3::xml_node_iso_639_3() :
    xml_node()
{
}


bool
xml_node_iso_639_3::is_a_valid_code(const nstring &code_name)
    const
{
    codes_t::const_iterator it = codes.find(code_name.downcase());
    return (it != codes.end());
}


void
xml_node_iso_639_3::assign(const nstring &name, const nstring &value)
{
    codes.insert(codes_t::value_type(name.downcase(), value));
}


bool
xml_node_iso_639_3::need_setup(void)
{
    if (codes.empty())
    {
        trace(("%s\n", __PRETTY_FUNCTION__));
        assign("en", "eng");
        assign("eng", "en");
        assign("c", "c");
        assign("posix", "posix");
        return true;
    }
    return false;
}


void
xml_node_iso_639_3::element_begin(const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    id.clear();
    part1_code.clear();
    part2_code.clear();
}


void
xml_node_iso_639_3::attribute(const nstring &name, const nstring &value)
{
    trace(("xml_node_iso_639_3::attribute(name = %s, value = %s)\n{\n",
        name.quote_c().c_str(), value.quote_c().c_str()));
    if (name == "id")
        id = value;
    else if (name == "part1_code")
        part1_code = value;
    else if (name == "part2_code")
        part2_code = value;
}


void
xml_node_iso_639_3::element_end(const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (part2_code.empty())
        part2_code = id;
    if (part1_code.empty())
    {
        if (part2_code.empty())
        {
            // do nothing with nothing
        }
        else
        {
            // only 3-letter code available
            assign(part2_code, part2_code);
        }
    }
    else
    {
        if (part2_code.empty())
        {
            // only 2-letter code available
            assign(part1_code, part1_code);
        }
        else
        {
            // map 2-letter to 3-letter codes
            assign(part1_code, part2_code);
            // map 3-letter to 2-letter codes
            assign(part2_code, part1_code);
        }
    }
}


// vim: set ts=8 sw=4 et :
