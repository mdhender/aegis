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

#ifndef LIBAEGIS_XML_NODE_RSS_TIMEDATE_H
#define LIBAEGIS_XML_NODE_RSS_TIMEDATE_H

#include <libaegis/xml_node/rss.h>

/**
  * The xml_node_rss_timedate class is used to represent a node processing
  * object for time/date elements of RSS channels (that is, pubDate and
  * lastBuildDate).
  *
  * Instantiating an object of this type will register itself with the
  * xml_text_reader_by_node object (specified in the constructor) as the
  * handler for the time/date elements.
  *
  * The time and date elements are updated with the current date/time or just
  * after midnight on the current date.
  */
class xml_node_rss_timedate:
    public xml_node_rss
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_rss_timedate();

    /**
     * Constructor.
     *
     * \param reader
     *     The xml_text_reader using this object.
     * \param out
     *     The output destination to write to.
     * \param use_current_time
     *     Boolean indicating whether or not the time portion of the
     *     date/time element should be set to just after midnight
     *     (false), or to the current time (true).
     */
    xml_node_rss_timedate(xml_text_reader_by_node &reader, output::pointer out,
	bool use_current_time);

    /**
      * The copy constructor.
      */
    xml_node_rss_timedate(const xml_node_rss_timedate &arg);

    /**
      * text handler
      */
    virtual void text(const nstring &name);

private:
    /**
     * Set the time to the current time?
     */
    bool set_current_time;

    /**
      * The default constructor.  Don't use.
      */
    xml_node_rss_timedate();

    /**
      * The assignment operator.  Don't use.
      */
    xml_node_rss_timedate &operator=(const xml_node_rss_timedate &);
};

#endif // LIBAEGIS_XML_NODE_RSS_TIMEDATE_H
