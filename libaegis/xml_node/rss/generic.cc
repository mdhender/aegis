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
#include <libaegis/xml_node/rss/generic.h>
#include <libaegis/xmltextread/by_node.h>


xml_node_rss_generic::~xml_node_rss_generic()
{
}


xml_node_rss_generic::xml_node_rss_generic(xml_text_reader_by_node &reader,
                                           output::pointer out) :
    xml_node_rss(reader, out)
{
    // Register all RSS elements that this class can handle.
    reader.register_node_handler("rss", *this);
    reader.register_node_handler("rss/channel/title", *this);
    reader.register_node_handler("rss/channel/link", *this);
    reader.register_node_handler("rss/channel/description", *this);
    reader.register_node_handler("rss/channel/language", *this);
    reader.register_node_handler("rss/channel/copyright", *this);
    reader.register_node_handler("rss/channel/managingEditor", *this);
    reader.register_node_handler("rss/channel/webMaster", *this);
    reader.register_node_handler("rss/channel/category", *this);
    reader.register_node_handler("rss/channel/generator", *this);
    reader.register_node_handler("rss/channel/docs", *this);
    reader.register_node_handler("rss/channel/cloud", *this);
    reader.register_node_handler("rss/channel/ttl", *this);
    reader.register_node_handler("rss/channel/rating", *this);
    reader.register_node_handler("rss/channel/textInput", *this);
    reader.register_node_handler("rss/channel/textInput/title", *this);
    reader.register_node_handler("rss/channel/textInput/description", *this);
    reader.register_node_handler("rss/channel/textInput/name", *this);
    reader.register_node_handler("rss/channel/textInput/link", *this);
    reader.register_node_handler("rss/channel/skipHours", *this);
    reader.register_node_handler("rss/channel/skipDays", *this);
    reader.register_node_handler("rss/channel/image", *this);
    reader.register_node_handler("rss/channel/image/url", *this);
    reader.register_node_handler("rss/channel/image/title", *this);
    reader.register_node_handler("rss/channel/image/link", *this);
    reader.register_node_handler("rss/channel/image/width", *this);
    reader.register_node_handler("rss/channel/image/height", *this);
    reader.register_node_handler("rss/channel/image/description", *this);
    reader.register_node_handler("rss/channel/item/title", *this);
    reader.register_node_handler("rss/channel/item/description", *this);
    reader.register_node_handler("rss/channel/item/pubDate", *this);
    reader.register_node_handler("rss/channel/item/link", *this);
    reader.register_node_handler("rss/channel/item/author", *this);
    reader.register_node_handler("rss/channel/item/category", *this);
    reader.register_node_handler("rss/channel/item/comments", *this);
    reader.register_node_handler("rss/channel/item/enclosure", *this);
    reader.register_node_handler("rss/channel/item/guid", *this);
    reader.register_node_handler("rss/channel/item/source", *this);
}


xml_node_rss_generic::xml_node_rss_generic(const xml_node_rss_generic &arg) :
    xml_node_rss(arg)
{
}


void
xml_node_rss_generic::text(const nstring &value)
{
    terminate_element_begin();

    //
    // The value string has had all of its XML entities (&lt; et al)
    // decoded, we must be sure to re-encode it...
    //
    deeper->fputs_xml(value);
}
