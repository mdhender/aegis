//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/xmltextread/by_node.h>
#include <libaegis/xml_node.h>


xml_text_reader_by_node::~xml_text_reader_by_node()
{
    trace(("~xml_text_reader_by_node()\n"));
}


xml_text_reader_by_node::xml_text_reader_by_node(input &arg1, bool arg2) :
    xml_text_reader(arg1, arg2),
    current_node(0)
{
    trace(("xml_text_reader_by_node()\n"));
}


void
xml_text_reader_by_node::register_node_handler(const nstring &hname,
    xml_node &handler)
{
    trace(("xml_text_reader_by_node::register_node_handler(name = \"%s\")\n",
	hname.c_str()));
    handlers.assign(hname.downcase(), &handler);
    calculate_current_node_handler();
    trace(("}\n"));
}


void
xml_text_reader_by_node::push_name(const nstring &arg)
{
    trace(("xml_text_reader_by_node::push_name(ar = \"%s\")\n{\n",
	arg.c_str()));
    if (names.empty())
	names.push_back(arg.downcase());
    else
	names.push_back(names.back() + "/" + arg.downcase());
    calculate_current_node_handler();
    trace(("}\n"));
}


void
xml_text_reader_by_node::pop_name(const nstring &arg)
{
    trace(("xml_text_reader_by_node::pop_name()\n{\n"));
    assert(!names.empty());
    (void)arg;
    assert
    (
	names.size() == 1
    ?
	names.back() == arg.downcase()
    :
	names[names.size() - 2] + "/" + arg.downcase() == names.back()
    );
    names.pop_back();
    calculate_current_node_handler();
    trace(("}\n"));
}


void
xml_text_reader_by_node::calculate_current_node_handler()
{
    trace(("xml_text_reader_by_node::calculate_current_node_handler()\n{\n"));
    if (names.empty())
	current_node = 0;
    else
	current_node = handlers.query(names.back());
    trace(("}\n"));
}


void
xml_text_reader_by_node::process_node()
{
    trace(("xml_text_reader_by_node::process_node()\n{\n"));
    const xmlChar *tmp = name();
    nstring the_name((const char *)tmp);
    if (tmp)
	xmlFree((void *)tmp);
    tmp = value();
    nstring the_value((const char *)tmp);
    if (tmp)
	xmlFree((void *)tmp);
    switch (node_type())
    {
    case XML_READER_TYPE_NONE:
	if (current_node)
	    current_node->none(the_name, the_value);
	break;

    case XML_READER_TYPE_ELEMENT:
	push_name(the_name);
	if (current_node)
	    current_node->element_begin(the_name);
	break;

    case XML_READER_TYPE_ATTRIBUTE:
	if (current_node)
	    current_node->attribute(the_name, the_value);
	break;

    case XML_READER_TYPE_TEXT:
	if (current_node)
	    current_node->text(the_value);
	break;

    case XML_READER_TYPE_CDATA:
	if (current_node)
	    current_node->cdata(the_value);
	break;

    case XML_READER_TYPE_ENTITY_REFERENCE:
	if (current_node)
	    current_node->entity_reference(the_name, the_value);
	break;

    case XML_READER_TYPE_ENTITY:
	push_name(the_name);
	if (current_node)
	{
	    current_node->entity_begin(the_name, the_value);
	    if (is_empty_element())
		current_node->entity_end(the_name, the_value);
	}
	break;

    case XML_READER_TYPE_PROCESSING_INSTRUCTION:
	if (current_node)
	    current_node->processing_instruction(the_name, the_value);
	break;

    case XML_READER_TYPE_COMMENT:
	if (current_node)
	    current_node->comment(the_value);
	break;

    case XML_READER_TYPE_DOCUMENT:
	if (current_node)
	    current_node->document(the_value);
	break;

    case XML_READER_TYPE_DOCUMENT_TYPE:
	if (current_node)
	    current_node->document_type(the_name, the_value);
	break;

    case XML_READER_TYPE_DOCUMENT_FRAGMENT:
	if (current_node)
	    current_node->document_fragment(the_value);
	break;

    case XML_READER_TYPE_NOTATION:
	if (current_node)
	    current_node->notation(the_name, the_value);
	break;

    case XML_READER_TYPE_WHITESPACE:
	if (current_node)
	    current_node->whitespace(the_value);
	break;

    case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
	if (current_node)
	    current_node->significant_whitespace(the_value);
	break;

    case XML_READER_TYPE_END_ELEMENT:
	if (current_node)
	    current_node->element_end(the_name);
	pop_name(the_name);
	break;

    case XML_READER_TYPE_END_ENTITY:
	if (current_node)
	    current_node->entity_end(the_name, the_value);
	pop_name(the_name);
	break;

    case XML_READER_TYPE_XML_DECLARATION:
	if (current_node)
	    current_node->xml_declaration(the_name, the_value);
	break;
    }
    trace(("}\n"));
}


void
xml_text_reader_by_node::process_node_end(bool was_empty)
{
    trace(("xml_text_reader_by_node::process_node_end(%d)\n{\n", was_empty));
    if (was_empty)
    {
	move_back_to_element();
	const xmlChar *tmp = name();
	nstring the_name((const char *)tmp);
	if (tmp)
	    xmlFree((void *)tmp);
	if (current_node)
	    current_node->element_end(the_name);
	pop_name(the_name);
    }
    trace(("}\n"));
}
