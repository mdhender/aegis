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
#include <libaegis/xml_node/rss/item.h>
#include <libaegis/xmltextread/by_node.h>


xml_node_rss_item::~xml_node_rss_item()
{
    new_item = 0;
}


xml_node_rss_item::xml_node_rss_item(rss_item *the_item,
                                     xml_text_reader_by_node &reader,
                                     output::pointer out) :
    xml_node_rss(reader, out),
    new_item(the_item),
    num_items_found(0)
{
    reader.register_node_handler("rss/channel/item", *this);
}


xml_node_rss_item::xml_node_rss_item(const xml_node_rss_item &arg) :
    xml_node_rss(arg),
    new_item(arg.new_item),
    num_items_found(0)
{
}


void
xml_node_rss_item::element_begin(const nstring &)
{
    ++num_items_found;

    // If a new item has not been passed in, we merely count items.
    // If a new item has been passed in and this is the first item we've
    // encountered, then we need to dump the new item before dumping the
    // item just found.
    if (new_item != 0)
    {
        if (1 == num_items_found)
        {
            // Dump the new item.
            new_item->print(deeper);
        }

        // Print an open element for the item we are now handling.
        deeper->fputs("<item>\n");
    }
}


void
xml_node_rss_item::element_end(const nstring &)
{
    // If we are not merely counting items, terminate the item element.
    if (new_item != 0)
    {
        deeper->fputs("</item>\n");
    }
}


int
xml_node_rss_item::num_items()
    const
{
    return num_items_found;
}
