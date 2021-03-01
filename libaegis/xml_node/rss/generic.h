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

#ifndef LIBAEGIS_XML_NODE_RSS_GENERIC_H
#define LIBAEGIS_XML_NODE_RSS_GENERIC_H

#include <libaegis/xml_node/rss.h>

/**
  * The xml_node_rss_generic class is used to represent a node processing
  * object for general elements of RSS feeds.
  *
  * Instantiating this object will cause it to register as a handler for most
  * RSS elements with the xmltextreader passed in via the constructor.
  *
  * When handled, each element is just printed to the specified output
  * destination.
  */
class xml_node_rss_generic:
    public xml_node_rss
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_rss_generic();

    /**
     * Constructor.
     *
     * See base class for details.
     */
    xml_node_rss_generic(xml_text_reader_by_node &reader, output::pointer out);

    /**
      * The copy constructor.
      */
    xml_node_rss_generic(const xml_node_rss_generic &arg);

    /**
     * text handler
     */
    virtual void text(const nstring &value);

    /**
      * The assignment operator.
      */
    xml_node_rss_generic &operator=(const xml_node_rss_generic &);

private:
    /**
      * The default constructor.  Don't use.
      */
    xml_node_rss_generic();
};

#endif // LIBAEGIS_XML_NODE_RSS_GENERIC_H
