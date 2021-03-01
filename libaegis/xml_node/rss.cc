//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee
//      Copyright (C) 2008 Peter Miller
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

#include <libaegis/output.h>
#include <libaegis/xml_node/rss.h>
#include <libaegis/xmltextread/by_node.h>


xml_node_rss::~xml_node_rss()
{
}


xml_node_rss::xml_node_rss(xml_text_reader_by_node &reader,
        const output::pointer &a_deeper) :
    xml_node(),
    xml_reader(reader),
    deeper(a_deeper),
    begin_element_terminated(false),
    xml_version_printed(false)
{
}


xml_node_rss::xml_node_rss(const xml_node_rss &arg) :
    xml_node(arg),
    xml_reader(arg.xml_reader),
    deeper(arg.deeper),
    begin_element_terminated(false),
    xml_version_printed(arg.xml_version_printed)
{
}


void
xml_node_rss::element_begin(const nstring &name)
{
    // Have we printed out the xml version element?
    if (!xml_version_printed)
    {
        deeper->fputs("<?xml version=\"1.0\"?>");
        deeper->end_of_line();
        xml_version_printed = true;
    }

    deeper->fputc('<');
    deeper->fputs(name);

    // Do not termintate the element in case it has an attribute attached.
    begin_element_terminated = false;
}


void
xml_node_rss::element_end(const nstring &name)
{
    deeper->fputs("</");
    deeper->fputs(name);
    deeper->fputs(">\n");
}


void
xml_node_rss::attribute(const nstring &name, const nstring &value)
{
    deeper->fputc(' ');
    deeper->fputs(name);
    deeper->fputs("=\"");
    deeper->fputs(value);
    deeper->fputc('"');
    terminate_element_begin();
}


void
xml_node_rss::terminate_element_begin()
{
    if (!begin_element_terminated)
    {
        deeper->fputc('>');
        begin_element_terminated = true;
    }
}
