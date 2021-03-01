//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_XMLTEXTREAD_H
#define LIBAEGIS_XMLTEXTREAD_H

#include <libxml/xmlreader.h>
#include <libxml/xmlversion.h>

#if LIBXML_VERSION < 20510
enum xmlReaderTypes
{
    XML_READER_TYPE_NONE = 0,
    XML_READER_TYPE_ELEMENT = 1,
    XML_READER_TYPE_ATTRIBUTE = 2,
    XML_READER_TYPE_TEXT = 3,
    XML_READER_TYPE_CDATA = 4,
    XML_READER_TYPE_ENTITY_REFERENCE = 5,
    XML_READER_TYPE_ENTITY = 6,
    XML_READER_TYPE_PROCESSING_INSTRUCTION = 7,
    XML_READER_TYPE_COMMENT = 8,
    XML_READER_TYPE_DOCUMENT = 9,
    XML_READER_TYPE_DOCUMENT_TYPE = 10,
    XML_READER_TYPE_DOCUMENT_FRAGMENT = 11,
    XML_READER_TYPE_NOTATION = 12,
    XML_READER_TYPE_WHITESPACE = 13,
    XML_READER_TYPE_SIGNIFICANT_WHITESPACE = 14,
    XML_READER_TYPE_END_ELEMENT = 15,
    XML_READER_TYPE_END_ENTITY = 16,
    XML_READER_TYPE_XML_DECLARATION = 17
};
#endif

#include <libaegis/input.h>

/**
  * The xml_text_reader class is used to represent an XML stream reader.
  * It uses the xmlTextReader* functions from the GNOME libxml2 library.
  */
class xml_text_reader
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_text_reader();

    /**
      * The constructor.
      */
    xml_text_reader(input &deeper, bool validate = false);

    /**
      * The parse method is used to parse the given file.  Each element
      * will cause the process_node(void) method to be called.
      */
    void parse(void);

protected:
    /**
      * The process_node method is used to process each node as it is
      * read from the input.  Each derived class must supply its own
      * process_node method.
      */
    virtual void process_node(void) = 0;

    /**
      * The process_node_end method is only ever called for ELEMENT
      * nodes, and only after all of the attributes have been given to
      * the process_node method.
      *
      * The default implimentation does nothing.
      *
      * @param empty
      *     Whether or not the original ELEMENT node was empty.
      */
    virtual void process_node_end(bool empty);

    /**
      * The depth method is used to obtain the depth of the current node
      * in the tree.
      *
      * @returns
      *     the depth or -1 in case of error
      */
    int depth(void) { return xmlTextReaderDepth(reader); }

    /**
      * The node_type method is used to get the node type of the current
      * node.
      *
      * @returns
      *     the xmlNodeType of the current node or -1 in case of error
      *
      * @sa
      *     http://dotgnu.org/pnetlib-doc/System/Xml/XmlNodeType.html
      */
    xmlReaderTypes
    node_type(void)
    {
        return (xmlReaderTypes)xmlTextReaderNodeType(reader);
    }

    /**
      * The xmlReaderType_ename class method is used to obtain the text
      * equivalent of a node type.
      */
    static const char *xmlReaderType_ename(xmlReaderTypes);

    /**
      * The name method is used to determine the qualified name of the
      * current node, equal to Prefix:LocalName.
      *
      * @returns
      *     the node name or NULL if not available
      */
    const xmlChar *name(void) { return xmlTextReaderName(reader); }

    /**
      * The is_empty_element method is used to check if the current node
      * is empty.
      *
      * @returns
      *     1 if empty, 0 if not and -1 in case of error
      */
    int is_empty_element(void) { return xmlTextReaderIsEmptyElement(reader); }

    /**
      * The has_value method is used to determine whether or not the
      * current node has a text value.
      *
      * @returns
      *     1 if true, 0 if false, and -1 in case or error
      */
    int has_value(void) { return xmlTextReaderHasValue(reader); }

    /**
      * The value method is used to provide the text value of the
      * current node if present.
      *
      * @returns
      *     the string or NULL if not available. The result must be
      *     deallocated with xmlFree()
      */
    const xmlChar *value(void) { return xmlTextReaderValue(reader); }

    /**
      * The move_back_to_element method is used after parsing the
      * element attributes to move the reader pointer back to the
      * element node once again so that the name is again available.
      */
    void move_back_to_element(void) { xmlTextReaderMoveToElement(reader); }

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * input stream which the XML parser will use to obtain its input.
      */
    input deeper;

    /**
      * The reader instance variable is used to remember the location of
      * the opaque xml_text_reader data store.
      */
    xmlTextReaderPtr reader;

    /**
      * The number_of_errors instance variable is used to remember how
      * many errors have been detected to date in the parsing of the
      * input stream.
      */
    int number_of_errors;

    /**
      * The number_of_parsers class variable is used to remember how
      * many instances are currently active.  This lets us initialise
      * and clean up after the xml2 library.
      */
    static int number_of_parsers;

    /**
      * The validate instance variable is used to remember whether or
      * not we are validating the data against the DTD.
      */
    bool validate;

    /**
      * The read_callback class method is used to handle callbacks by
      * the parser.  It calls the read_deeper method.
      */
    static int read_callback(void *context, char *buffer, int len);

    /**
      * The read_deeper method is used by the read_callback class method
      * is used to obtain the next block of input.
      */
    int read_deeper(char *buffer, int len);

    /**
      * The close_callback class method is used to handle callbacks by
      * the parser when end of inpout is reached.
      */
    static int close_callback(void *context);

    /**
      * The close method is called by the parser when it gets to the end
      * of the input.
      */
    void close(void);

    /**
      * The error_callback class method is used to handle callbacks by
      * the parser when an error is seen.
      */
    static void error_callback(void *context, const char *msg,
        xmlParserSeverities severity, xmlTextReaderLocatorPtr locator);

    /**
      * the error method is used to report errors and warnings detected
      * during the parse.
      */
    void error(const char *msg, xmlParserSeverities severity,
        xmlTextReaderLocatorPtr locator);

    /**
      * The read method is used to read one node from the input, and
      * process it via the virtual process_node method.  If there is
      * an error, it is reported in a fatal error message, and this
      * function does not return.
      *
      * @returns
      *     If end of file is reached, false is returned.
      *     If there is not error, true is returned.
      */
    bool read(void);

    /**
      * The default constructor.  Do not use.
      */
    xml_text_reader();

    /**
      * The copy constructor.  Do not use.
      */
    xml_text_reader(const xml_text_reader &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_text_reader &operator=(const xml_text_reader &);
};

#endif // LIBAEGIS_XMLTEXTREAD_H
// vim: set ts=8 sw=4 et :
