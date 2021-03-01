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
// MANIFEST: implementation of the xml_node_rss_item class
//

#pragma implementation "xml_node_rss_item"

#include <output.h>
#include <rss/item.h>
#include <xml_node/rss/item.h>
#include <xmltextread/by_node.h>


xml_node_rss_item::~xml_node_rss_item()
{
    new_item = 0;
}


xml_node_rss_item::xml_node_rss_item(rss_item *the_item,
                                     xml_text_reader_by_node &reader,
                                     output_ty *out) :
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
xml_node_rss_item::element_begin(const nstring &name)
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
            new_item->print(output);
        }

        // Print an open element for the item we are now handling.
        output->fputs("<item>\n");
    }
}


void
xml_node_rss_item::element_end(const nstring &name)
{
    // If we are not merely counting items, terminate the item element.
    if (new_item != 0)
    {
        output->fputs("</item>\n");
    }
}


int
xml_node_rss_item::num_items()
    const
{
    return num_items_found;
}
