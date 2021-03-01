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

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <libaegis/output.h>
#include <test_xml/xml_node/dump.h>


xml_node_dump::~xml_node_dump()
{
}


xml_node_dump::xml_node_dump(const output::pointer &arg) :
    op(arg)
{
}


static void
c_escape(output::pointer op, const char *s, int len)
{
    while (len-- > 0)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case 0:
	    return;

	case '\r':
	    op->fputs("\\r");
	    break;

	case '\n':
	    op->fputs("\\n");
	    break;

	case '\t':
	    op->fputs("\\t");
	    break;

	case '\b':
	    op->fputs("\\b");
	    break;

	case '\f':
	    op->fputs("\\f");
	    break;

	case '"':
	case '\\':
	    op->fputc('\\');
	    op->fputc(c);
	    break;

	default:
	    // C locale
	    if (isprint(c))
		op->fputc(c);
	    else
		op->fprintf("\\%03o", c);
	    break;
	}
    }
}


static void
c_escape(output::pointer op, const nstring &s)
{
    c_escape(op, s.c_str(), s.size());
}


static void
print_one(output::pointer op, const char *caption, const nstring &name,
    const nstring &value)
{
    op->fputs(caption);
    op->fputs(":");
    bool comma = false;
    if (!name.empty())
    {
	op->fputs(" name = \"");
	c_escape(op, name);
	op->fputc('"');
	comma = true;
    }
    if (!value.empty())
    {
	if (comma)
		op->fputc(',');
	op->fputs(" value = \"");
	c_escape(op, value);
	op->fputc('"');
    }
    op->fputc('\n');
}


void
xml_node_dump::none(const nstring &name, const nstring &value)
{
    print_one(op, "none", name, value);
}


void
xml_node_dump::attribute(const nstring &name, const nstring &value)
{
    print_one(op, "attribute", name, value);
}


void
xml_node_dump::comment(const nstring &value)
{
    print_one(op, "comment", "", value);
}


void
xml_node_dump::document(const nstring &value)
{
    print_one(op, "document", "", value);
}


void
xml_node_dump::document_fragment(const nstring &value)
{
    print_one(op, "document fragment", "", value);
}


void
xml_node_dump::document_type(const nstring &name, const nstring &value)
{
    print_one(op, "document type", name, value);
}


void
xml_node_dump::element_begin(const nstring &name)
{
    print_one(op, "element begin", name, "");
}


void
xml_node_dump::element_end(const nstring &name)
{
    print_one(op, "element end", name, "");
}


void
xml_node_dump::entity_begin(const nstring &name, const nstring &value)
{
    print_one(op, "entity begin", name, value);
}


void
xml_node_dump::entity_end(const nstring &name, const nstring &value)
{
    print_one(op, "entity end", name, value);
}


void
xml_node_dump::entity_reference(const nstring &name, const nstring &value)
{
    print_one(op, "entity reference", name, value);
}


void
xml_node_dump::notation(const nstring &name, const nstring &value)
{
    print_one(op, "notation", name, value);
}


void
xml_node_dump::processing_instruction(const nstring &name, const nstring &value)
{
    print_one(op, "processing instruction", name, value);
}


void
xml_node_dump::text(const nstring &value)
{
    print_one(op, "text", "", value);
}


void
xml_node_dump::whitespace(const nstring &value)
{
    print_one(op, "whitespace", "", value);
}


void
xml_node_dump::xml_declaration(const nstring &name, const nstring &value)
{
    print_one(op, "xml declaration", name, value);
}
