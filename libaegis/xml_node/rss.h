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

#ifndef LIBAEGIS_XML_NODE_RSS_H
#define LIBAEGIS_XML_NODE_RSS_H

#include <libaegis/xml_node.h>

// Forward declarations
class output_ty;
class xml_text_reader_by_node;

/**
  * The xml_node_rss class is a base class for specific RSS node processors.
  * It exists merely to group together attributes common to the RSS
  * processors.
  *
  * Objects of this class cannot be instantiated.
  */
class xml_node_rss:
    public xml_node
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_rss();

    /**
      * The copy constructor.
      */
    xml_node_rss(const xml_node_rss &arg);

    /**
      * element_begin handler.  Write out the element.
      */
    virtual void element_begin(const nstring &name);

    /**
      * element_end handler.  Write out the element.
      */
    virtual void element_end(const nstring &name);

    /**
      * attribute handler.  Write out the attribute and value.
      */
    virtual void attribute(const nstring &name, const nstring &value);

protected:
    /**
      * Constructor.
      *
      * Protected so this base class cannot be instantiated.
      *
      * \param reader
      *     The xml_text_reader_by_node using this object.
      * \param out
      *     The output destination to write to.
      */
    xml_node_rss(xml_text_reader_by_node &reader, const output::pointer &out);

    /**
      * Take care of terminating a begin element if necessary.
      */
    void terminate_element_begin();

    /**
      * The xmltextreader that is using this object.  Store its address so
      * we can register ourselves to handle a whole bunch of RSS elements.
      */
    const xml_text_reader_by_node &xml_reader;

    /**
      * The output destination to write to.
      */
    output::pointer deeper;

    /**
      * Remember if the begin element has been properly terminated with a
      * trailing '>'.
      *
      * The element_begin handler cannot close the element when it prints it
      * out in case that element contains an attribute.  So the opening element
      * must be left unterminated until it is determined if there is an
      * attribute present or not.
      */
    bool begin_element_terminated;

    /**
      * We need to print out the XML version as the first line of output.
      * This doesn't seem to be handled by the xml_text_reader_by_node.
      */
    bool xml_version_printed;

private:
    /**
     * The default constructor.  Don't use.
     */
    xml_node_rss();

    /**
      * The assignment operator.  Don't use.
      */
    xml_node_rss &operator=(const xml_node_rss &);
};

#endif // LIBAEGIS_XML_NODE_RSS_H
