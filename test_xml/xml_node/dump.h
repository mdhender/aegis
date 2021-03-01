//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the xml_node_dump class
//

#ifndef TEST_XML_XML_NODE_DUMP_H
#define TEST_XML_XML_NODE_DUMP_H

#include <libaegis/xml_node.h>

class output; // forward

/**
  * The xml_node_dump class is used to represent an xml processing node
  * which simply prints the arguments to the methods, for debugging.
  */
class xml_node_dump:
    public xml_node
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_dump();

    /**
      * The constructor.
      */
    xml_node_dump(output_ty *op);

protected:
    // See base class for documentation.
    void none(const nstring &name, const nstring &value);

    // See base class for documentation.
    void attribute(const nstring &name, const nstring &value);

    // See base class for documentation.
    void comment(const nstring &value);

    // See base class for documentation.
    void document(const nstring &value);

    // See base class for documentation.
    void document_fragment(const nstring &value);

    // See base class for documentation.
    void document_type(const nstring &name, const nstring &value);

    // See base class for documentation.
    void element_begin(const nstring &name);

    // See base class for documentation.
    void element_end(const nstring &name);

    // See base class for documentation.
    void entity_begin(const nstring &name, const nstring &value);

    // See base class for documentation.
    void entity_end(const nstring &name, const nstring &value);

    // See base class for documentation.
    void entity_reference(const nstring &name, const nstring &value);

    // See base class for documentation.
    void notation(const nstring &name, const nstring &value);

    // See base class for documentation.
    void processing_instruction(const nstring &name, const nstring &value);

    // See base class for documentation.
    void text(const nstring &value);

    // See base class for documentation.
    void whitespace(const nstring &value);

    // See base class for documentation.
    void xml_declaration(const nstring &name, const nstring &value);

private:
    /**
      * The op instance variable is used to remember where to send the
      * output.
      */
    output_ty *op;

    /**
      * The default constructor.  Do not use.
      */
    xml_node_dump();

    /**
      * The copy constructor.  Do not use.
      */
    xml_node_dump(const xml_node_dump &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_node_dump &operator=(const xml_node_dump &);
};

#endif // TEST_XML_XML_NODE_DUMP_H
