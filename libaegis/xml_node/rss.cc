//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the xml_node_rss class
//

#pragma implementation "xml_node_rss"

#include <output.h>
#include <xml_node/rss.h>
#include <xmltextread/by_node.h>


xml_node_rss::~xml_node_rss()
{
    output = 0;
}


xml_node_rss::xml_node_rss(xml_text_reader_by_node &reader, output_ty *out) :
    xml_node(),
    xml_reader(reader),
    output(out),
    begin_element_terminated(false),
    xml_version_printed(false)
{
}


xml_node_rss::xml_node_rss(const xml_node_rss &arg) :
    xml_node(arg),
    xml_reader(arg.xml_reader),
    output(arg.output),
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
        output->fputs("<?xml version=\"1.0\"?>");
        output->end_of_line();
        xml_version_printed = true;
    }

    output->fputc('<');
    output->fputs(name);

    // Do not termintate the element in case it has an attribute attached.
    begin_element_terminated = false;
}


void
xml_node_rss::element_end(const nstring &name)
{
    output->fputs("</");
    output->fputs(name);
    output->fputs(">\n");
}


void
xml_node_rss::attribute(const nstring &name, const nstring &value)
{
    output->fputc(' ');
    output->fputs(name);
    output->fputs("=\"");
    output->fputs(value);
    output->fputc('"');
    terminate_element_begin();
}


void
xml_node_rss::terminate_element_begin()
{
    if (!begin_element_terminated)
    {
        output->fputc('>');
        begin_element_terminated = true;
    }
}
