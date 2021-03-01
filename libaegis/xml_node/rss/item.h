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

#ifndef LIBAEGIS_XML_NODE_RSS_ITEM_H
#define LIBAEGIS_XML_NODE_RSS_ITEM_H

#include <libaegis/xml_node/rss.h>

// Forward declarations
class rss_item;

/**
  * The xml_node_rss_item class is used to represent a node processing object
  * for item elements of RSS channels.
  *
  * Instantiating an object of this type will register itself with the
  * xml_text_reader_by_node object (specified in the constructor) as the
  * handler for the item element (not item sub-elements).
  *
  * This class counts items as they are encounted in an RSS feed file.  If
  * no new item (to be added to the RSS feed) is specified, then that is all
  * this class does.  If a new item is specified, that new item will be
  * added to the feed <b>before</b> any existing items.
  *
  * This class can thus be used for two purposes -- for counting items and
  * for adding a new item.  If it is used to add a new item and no existing
  * items are found in the feed, that new item will not be added.
  */
class xml_node_rss_item:
    public xml_node_rss
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_rss_item();

    /**
      * Constructor.
      *
      * \param the_item
      *     The item to the added.  Can be null (see above).
      * \param reader
      *     The xml_text_reader using this object.
      * \param out
      *     The output destination to write to.
      */
    xml_node_rss_item(rss_item *the_item, xml_text_reader_by_node &reader,
	output::pointer out);

    /**
      * The copy constructor.
      */
    xml_node_rss_item(const xml_node_rss_item &);

    /**
     * Handle element beginings.
     */
    virtual void element_begin(const nstring &name);

    /**
     * Handle element endings.
     */
    virtual void element_end(const nstring &name);

    /**
     * Get the number of item elements encountered.
     */
    int num_items() const;

private:
    /**
     * The new item to be added to the feed.
     */
    rss_item *new_item;

    /**
     * The number of items encountered.
     */
    int num_items_found;

    /**
      * The default constructor.  Don't use.
      */
    xml_node_rss_item();

    /**
      * The assignment operator.  Don't use.
      */
    xml_node_rss_item &operator=(const xml_node_rss_item &);
};

#endif // LIBAEGIS_XML_NODE_RSS_ITEM_H
