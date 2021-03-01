//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_XMLTEXTREAD_BY_NODE_H
#define LIBAEGIS_XMLTEXTREAD_BY_NODE_H

#include <common/symtab/template.h>
#include <libaegis/xmltextread.h>

class xml_node; // forward

/**
  * The xml_text_reader_by_node class is used to represent an XML text
  * reader which dispatches nodes to their registered handlers.
  */
class xml_text_reader_by_node:
    public xml_text_reader
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_text_reader_by_node();

    /**
      * The constructor.
      */
    xml_text_reader_by_node(input &deeper, bool validate = false);

    /**
      * The register_node_handler method is sued to register a node name
      * and a corresponding handler.
      *
      * @param name
      *     The name of the node to be handled.  Nodes are named ratyher
      *     like UNIX paths, they are slash separated.  E.g. parent/
      *     child/ subchild, etc.  NO wildcards are available.
      * @param handler
      *     The object which will adela with the named nodes.  Noth that
      *     (depending on the DTD) this may be called from more than one
      *     instance, but always with begin/end pairs, so it is possible
      *     to keep track.
      */
    void register_node_handler(const nstring &name, xml_node &handler);

    // See base class for documentation.
    void process_node();

    // See base class for documentation.
    void process_node_end(bool empty);

private:
    /**
      * The handlers instance variable is used to remember what handler
      * have been registered for what nodes.
      */
    symtab<xml_node> handlers;

    /**
      * The name instance variable is used to rememeber the stack of
      * names (elements and entities) seen to date.
      */
    nstring_list names;

    /**
      * The current_node instance variable is used to cache the node
      * handler corresponding to the name at the top of the name stack.
      */
    xml_node *current_node;

    /**
      * the push_name method is used to tack action when the beginning
      * of an entity or element is seen.
      */
    void push_name(const nstring &name);

    /**
      * the pop_name method is used to tack action when the end of an
      * entity or element is seen.
      */
    void pop_name(const nstring &name);

    /**
      * The calculate_current_node_handler method is used to set the
      * current_node from the name at the top of the names stack.
      */
    void calculate_current_node_handler();

    /**
      * The default constructor.  Do not use.
      */
    xml_text_reader_by_node();

    /**
      * The copy constructor.  Do not use.
      */
    xml_text_reader_by_node(const xml_text_reader_by_node &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_text_reader_by_node &operator=(const xml_text_reader_by_node &);
};

#endif // LIBAEGIS_XMLTEXTREAD_BY_NODE_H
