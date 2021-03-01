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

#ifndef LIBAEGIS_XML_NODE_RSS_CHANNEL_H
#define LIBAEGIS_XML_NODE_RSS_CHANNEL_H

#include <libaegis/xml_node/rss.h>

// Forward declarations
class rss_item;

/**
  * The xml_node_rss_channel class is used to represent a node processing
  * object for channel elements of RSS feeds.
  *
  * Instantiating an object of this class will register itself with the
  * xml_text_reader_by_node object (specified in the constructor) as the
  * handler for the channel element (not channel sub-elements).
  *
  * An RSS item is also specified.  This is an item intended to be added
  * to an RSS feed file.  The new item's contents will be dumped to the
  * specified output when the channel end element is handled.
  *
  * This class is used to print channel element begin/ends, but also new items
  * when we are dealing with an RSS feed that contains exactly zero items
  * already.  Alternatively, it could be used for feeds that contains items,
  * and we want the new item to be added to the <b>end</b> of the list of
  * existing items.  This would probably be unusual, though.
  */
class xml_node_rss_channel:
    public xml_node_rss
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_rss_channel();

    /**
      * Constructor.
      *
      * \param the_item
      *     The item to the added.  Can be null, in which case only the
      *     channel element begin/end are printed.
      * \param reader
      *     The xml_text_reader using this object.
      * \param out
      *     The output destination to write to.
      */
    xml_node_rss_channel(rss_item *the_item, xml_text_reader_by_node &reader,
	output::pointer out);

    /**
      * The copy constructor.
      */
    xml_node_rss_channel(const xml_node_rss_channel &);

    /**
     * Handle element beginings.
     */
    virtual void element_begin(const nstring &name);

    /**
     * Handle element endings.
     */
    virtual void element_end(const nstring &name);

private:
    /**
     * The new item.
     */
    rss_item *new_item;

    /**
      * The default constructor.  Don't use.
      */
    xml_node_rss_channel();

    /**
      * The assignment operator.  Don't use.
      */
    xml_node_rss_channel &operator=(const xml_node_rss_channel &);
};

#endif // LIBAEGIS_XML_NODE_RSS_CHANNEL_H
