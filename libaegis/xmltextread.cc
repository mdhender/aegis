//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/input.h>
#include <libaegis/sub.h>
#include <libaegis/xmltextread.h>


int xml_text_reader::number_of_parsers;


xml_text_reader::~xml_text_reader()
{
    if (reader)
        xmlFreeTextReader(reader);
    reader = 0;

    //
    // We may need to cleanup after xml_text_reader
    //
    --number_of_parsers;
    if (number_of_parsers == 0)
        xmlCleanupParser();

    //
    // Print an error summary and fail with non-zero exit status if
    // there have been any errors during the parse.
    //
    close();
    deeper.close();
}


xml_text_reader::xml_text_reader(input &arg1, bool arg2) :
    deeper(arg1),
    reader(0),
    number_of_errors(0),
    validate(arg2)
{
    if (number_of_parsers == 0)
    {
        //
        // This initialize the library and check potential ABI
        // mismatches between the version it was compiled for and the
        // actual shared library used.
        //
        LIBXML_TEST_VERSION;
    }

    //
    // Keep track of how many parsers are using the library,
    // because we have to clean up after the last one.
    //
    ++number_of_parsers;
}


int
xml_text_reader::read_callback(void *context, char *buffer, int len)
{
    xml_text_reader *p = (xml_text_reader *)context;
    return p->read_deeper(buffer, len);
}


int
xml_text_reader::read_deeper(char *buffer, int len)
{
    if (!deeper.is_open())
        return 0;
    return deeper->read(buffer, len);
}


int
xml_text_reader::close_callback(void *context)
{
    xml_text_reader *p = (xml_text_reader *)context;
    p->close();
    return 0;
}


void
xml_text_reader::close()
{
    if (number_of_errors)
    {
        assert(deeper.is_open());
        sub_context_ty sc;
        sc.var_set_string("File_Name", deeper->name());
        sc.var_set_long("Number", number_of_errors);
        sc.var_optional("Number");
        sc.fatal_intl(i18n("$filename: has errors"));
        // NOTREACHED
    }
}


void
xml_text_reader::error_callback(void *context, const char *msg,
    xmlParserSeverities severity, xmlTextReaderLocatorPtr locator)
{
    xml_text_reader *p = (xml_text_reader *)context;
    p->error(msg, severity, locator);
}


void
xml_text_reader::error(const char *msg, xmlParserSeverities severity,
    xmlTextReaderLocatorPtr locator)
{
    assert(deeper.is_open());
    sub_context_ty sc;
    switch (severity)
    {
    case XML_PARSER_SEVERITY_VALIDITY_WARNING:
    case XML_PARSER_SEVERITY_WARNING:
        sc.var_set_format("MeSsaGe", "warning: %s", msg);
        break;

    case XML_PARSER_SEVERITY_VALIDITY_ERROR:
    case XML_PARSER_SEVERITY_ERROR:
        ++number_of_errors;
        sc.var_set_charstar("MeSsaGe", msg);
        break;
    }
    const char *file_name = (const char *)xmlTextReaderLocatorBaseURI(locator);
    if (!file_name)
        file_name = deeper->name().c_str();
    int line_number = xmlTextReaderLocatorLineNumber(locator);
    sc.var_set_format("File_Name", "%s: %d", file_name, line_number);
    sc.error_intl(i18n("$filename: $message"));
}


#ifndef HAVE_XMLREADERFORIO

static xmlTextReaderPtr
xmlReaderForIO(xmlInputReadCallback ioread, xmlInputCloseCallback ioclose,
    void *ioctx, const char *url, const char *encoding, int options)
{
    if (!ioread)
        return 0;
    xmlParserInputBufferPtr input =
        xmlParserInputBufferCreateIO
        (
            ioread,
            ioclose,
            ioctx,
            XML_CHAR_ENCODING_NONE
        );
    if (!input)
        return 0;
    xmlTextReaderPtr reader = xmlNewTextReader(input, url);
    if (!reader)
    {
        xmlFreeParserInputBuffer(input);
        return 0;
    }
    // reader->allocs |= XML_TEXTREADER_INPUT;
    // xmlTextReaderSetup(reader, 0, url, encoding, options);
    return reader;
}

#endif // !HAVE_XMLREADERFORIO


bool
xml_text_reader::read()
{
    int ret = xmlTextReaderRead(reader);
    if (ret < 0)
    {
        if (number_of_errors)
        {
            assert(deeper.is_open());
            sub_context_ty sc;
            sc.var_set_string("File_Name", deeper->name());
            sc.var_set_long("Number", number_of_errors);
            sc.var_optional("Number");
            sc.fatal_intl(i18n("$filename: has errors"));
            // NOTREACHED
        }
        sub_context_ty sc;
        string_ty *message = sc.subst_intl(i18n("syntax error"));
        if (!deeper.is_open())
            sc.var_set_charstar("File_Name", "no file name available");
        else
            sc.var_set_string("File_Name", deeper->name());
        sc.var_set_string("MeSsaGe", message);
        sc.fatal_intl(i18n("$filename: $message"));
        // NOTREACHED
    }
    if (ret == 0)
        return false;
    trace(("node type = %s\n", xmlReaderType_ename(node_type())));
    process_node();
    if (node_type() == XML_READER_TYPE_ELEMENT)
    {
        bool was_empty = is_empty_element();
        if (xmlTextReaderMoveToFirstAttribute(reader) > 0)
        {
            for (;;)
            {
                process_node();
                if (xmlTextReaderMoveToNextAttribute(reader) <= 0)
                    break;
            }
        }
        process_node_end(was_empty);
    }
    return true;
}


void
xml_text_reader::process_node_end(bool)
{
    // Do nothing.
}


void
xml_text_reader::parse()
{
    trace(("xml_text_reader::parse()\n{\n"));
    if (!reader)
    {
        reader =
            xmlReaderForIO
            (
                read_callback,
                close_callback,
                this, // context
                deeper->name().c_str(), // url
                NULL, // encoding
                XML_PARSER_DEFAULTATTRS | XML_PARSER_SUBST_ENTITIES
            );
        if (!reader)
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", deeper->name());
            sc.var_set_charstar("MeSsaGe", "xmlReaderForIO");
            sc.fatal_intl(i18n("$filename: $message"));
            // NOTREACHED
        }

        //
        // Set the error handler so that the usual Aegis error handling
        // can happen, rather than use the libxml2 library's default.
        //
        xmlTextReaderSetErrorHandler(reader, error_callback, this);

        xmlTextReaderSetParserProp(reader, XML_PARSER_LOADDTD, 1);
        xmlTextReaderSetParserProp(reader, XML_PARSER_SUBST_ENTITIES, 1);
        if (validate)
            xmlTextReaderSetParserProp(reader, XML_PARSER_VALIDATE, 1);
    }

    //
    // Read the file, calling process_node for each element in the file.
    //
    while (read())
        ;
    trace(("}\n"));
}


const char *
xml_text_reader::xmlReaderType_ename(xmlReaderTypes n)
{
    // Why isn't this functionality part of the libxml2 API?
    switch (n)
    {
    case XML_READER_TYPE_NONE:
        return "XML_READER_TYPE_NONE";

    case XML_READER_TYPE_ELEMENT:
        return "XML_READER_TYPE_ELEMENT";

    case XML_READER_TYPE_ATTRIBUTE:
        return "XML_READER_TYPE_ATTRIBUTE";

    case XML_READER_TYPE_TEXT:
        return "XML_READER_TYPE_TEXT";

    case XML_READER_TYPE_CDATA:
        return "XML_READER_TYPE_CDATA";

    case XML_READER_TYPE_ENTITY_REFERENCE:
        return "XML_READER_TYPE_ENTITY_REFERENCE";

    case XML_READER_TYPE_ENTITY:
        return "XML_READER_TYPE_ENTITY";

    case XML_READER_TYPE_PROCESSING_INSTRUCTION:
        return "XML_READER_TYPE_PROCESSING_INSTRUCTION";

    case XML_READER_TYPE_COMMENT:
        return "XML_READER_TYPE_COMMENT";

    case XML_READER_TYPE_DOCUMENT:
        return "XML_READER_TYPE_DOCUMENT";

    case XML_READER_TYPE_DOCUMENT_TYPE:
        return "XML_READER_TYPE_DOCUMENT_TYPE";

    case XML_READER_TYPE_DOCUMENT_FRAGMENT:
        return "XML_READER_TYPE_DOCUMENT_FRAGMENT";

    case XML_READER_TYPE_NOTATION:
        return "XML_READER_TYPE_NOTATION";

    case XML_READER_TYPE_WHITESPACE:
        return "XML_READER_TYPE_WHITESPACE";

    case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
        return "XML_READER_TYPE_SIGNIFICANT_WHITESPACE";

    case XML_READER_TYPE_END_ELEMENT:
        return "XML_READER_TYPE_END_ELEMENT";

    case XML_READER_TYPE_END_ENTITY:
        return "XML_READER_TYPE_END_ENTITY";

    case XML_READER_TYPE_XML_DECLARATION:
        return "XML_READER_TYPE_XML_DECLARATION";
    }
    static char buffer[10];
    snprintf(buffer, sizeof(buffer), "unknown(%d)", n);
    return buffer;
}


// vim: set ts=8 sw=4 et :
