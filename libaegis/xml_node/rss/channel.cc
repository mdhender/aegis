//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee
//      Copyright (C) 2007, 2008 Peter Miller
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
#include <libaegis/rss/item.h>
#include <libaegis/xml_node/rss/channel.h>
#include <libaegis/xmltextread/by_node.h>


xml_node_rss_channel::~xml_node_rss_channel()
{
    new_item = 0;
}


xml_node_rss_channel::xml_node_rss_channel(rss_item *the_item,
                                           xml_text_reader_by_node &reader,
                                           output::pointer out) :
    xml_node_rss(reader, out),
    new_item(the_item)
{
    reader.register_node_handler("rss/channel", *this);
}


xml_node_rss_channel::xml_node_rss_channel(const xml_node_rss_channel &arg) :
    xml_node_rss(arg),
    new_item(arg.new_item)
{
}


void
xml_node_rss_channel::element_begin(const nstring &)
{
    // Just print out the element beginning.
    deeper->fputs("<channel>\n");
}


void
xml_node_rss_channel::element_end(const nstring &)
{
    // First, dump the new item, then close the channel element.
    if (new_item != 0)
    {
        new_item->print(deeper);
    }
    deeper->fputs("</channel>\n");
}
