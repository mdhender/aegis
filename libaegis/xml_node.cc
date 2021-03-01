//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <libaegis/xml_node.h>


xml_node::~xml_node()
{
}


xml_node::xml_node()
{
}


xml_node::xml_node(const xml_node &)
{
}


xml_node &
xml_node::operator=(const xml_node &)
{
    return *this;
}


void
xml_node::none(const nstring &, const nstring &)
{
}


void
xml_node::attribute(const nstring &, const nstring &)
{
}


void
xml_node::cdata(const nstring &value)
{
    text(value);
}


void
xml_node::comment(const nstring &)
{
}


void
xml_node::document(const nstring &)
{
}


void
xml_node::document_fragment(const nstring &)
{
}


void
xml_node::document_type(const nstring &, const nstring &)
{
}


void
xml_node::element_begin(const nstring &)
{
}


void
xml_node::element_end(const nstring &)
{
}


void
xml_node::entity_begin(const nstring &, const nstring &)
{
}


void
xml_node::entity_end(const nstring &, const nstring &)
{
}


void
xml_node::entity_reference(const nstring &, const nstring &)
{
}


void
xml_node::notation(const nstring &, const nstring &)
{
}


void
xml_node::processing_instruction(const nstring &, const nstring &)
{
}


void
xml_node::significant_whitespace(const nstring &value)
{
    text(value);
}


void
xml_node::text(const nstring &)
{
}


void
xml_node::whitespace(const nstring &)
{
}


void
xml_node::xml_declaration(const nstring &, const nstring &)
{
}
