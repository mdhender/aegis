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

#ifndef LIBAEGIS_XML_NODE_H
#define LIBAEGIS_XML_NODE_H

class nstring; // forward

/**
  * The xml_node class is used to represent an abstract XML node
  * processing object.  It's methods are called by an XML parse at the
  * appropriate times.
  */
class xml_node
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node();

    /**
      * The default constructor.
      */
    xml_node();

    /**
      * The copy constructor.
      */
    xml_node(const xml_node &);

    /**
      * The assignment operator.
      */
    xml_node &operator=(const xml_node &);

    /**
      * The none method is called for XML_READER_TYPE_NONE nodes.  The
      * default action is to do nothing.
      */
    virtual void none(const nstring &name, const nstring &value);

    /**
      * The attribute method is called for XML_READER_TYPE_ATTRIBUTE
      * nodes.  The default action is to do nothing.
      */
    virtual void attribute(const nstring &name, const nstring &value);

    /**
      * The cdata method is called for XML_READER_TYPE_ATTRIBUTE nodes.
      * The default action is to call the text method.
      */
    virtual void cdata(const nstring &value);

    /**
      * The attribute method is called for XML_READER_TYPE_COMMENT
      * nodes.  The default action is to do nothing.
      */
    virtual void comment(const nstring &value);

    /**
      * The document method is called for XML_READER_TYPE_DOCUMENT
      * nodes.  The default action is to do nothing.
      */
    virtual void document(const nstring &value);

    /**
      * The document_fragment method is called for XML_READER_TYPE_
      * DOCUMENT_FRAGMENT nodes.  The default action is to do nothing.
      */
    virtual void document_fragment(const nstring &value);

    /**
      * The document_type method is called for XML_READER_TYPE_DOCUMENT_
      * TYPE nodes.  The default action is to do nothing.
      */
    virtual void document_type(const nstring &name, const nstring &value);

    /**
      * The element_begin method is called for XML_READER_TYPE_ELEMENT
      * nodes.  The default action is to do nothing.
      */
    virtual void element_begin(const nstring &name);

    /**
      * The element_end method is called for XML_READER_TYPE_END_ELEMENT
      * nodes.  The default action is to do nothing.
      */
    virtual void element_end(const nstring &name);

    /**
      * The entity_begin method is called for XML_READER_TYPE_ENTITY
      * nodes.  The default action is to do nothing.
      */
    virtual void entity_begin(const nstring &name, const nstring &value);

    /**
      * The entity_end method is called for XML_READER_TYPE_END_ENTITY
      * nodes.  The default action is to do nothing.
      */
    virtual void entity_end(const nstring &name, const nstring &value);

    /**
      * The entity_reference method is called for XML_READER_TYPE_ENTITY
      * REFERENCE nodes.  The default action is to do nothing.
      */
    virtual void entity_reference(const nstring &name, const nstring &value);

    /**
      * The notation method is called for XML_READER_TYPE_NOTATION
      * nodes.  The default action is to do nothing.
      */
    virtual void notation(const nstring &name, const nstring &value);

    /**
      * The processing_instruction method is called for XML_READER_TYPE_
      * PROCESSING_INSTRUCTION nodes.  The default action is to do
      * nothing.
      */
    virtual void processing_instruction(const nstring &name,
    	const nstring &value);

    /**
      * The significant_whitespace method is called for XML_READER_TYPE_
      * SIGNIFICANT_WHITESPACE nodes.  The default action is to call
      * the text method.
      */
    virtual void significant_whitespace(const nstring &value);

    /**
      * The text method is called for XML_READER_TYPE_TEXT nodes.  The
      * default action is to do nothing.
      */
    virtual void text(const nstring &value);

    /**
      * The whitespace method is called for XML_READER_TYPE_WHITESPACE
      * nodes.  The default action is to do nothing.
      */
    virtual void whitespace(const nstring &value);

    /**
      * The xml_declaration method is called for XML_READER_TYPE_XML
      * DECLARATION nodes.  The default action is to do nothing.
      */
    virtual void xml_declaration(const nstring &name, const nstring &value);
};

#endif // LIBAEGIS_XML_NODE_H
