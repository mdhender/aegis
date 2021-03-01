//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee;
//      Copyright (C) 2006-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/gettime.h>
#include <common/mem.h>
#include <common/now.h>
#include <common/nstring.h>
#include <common/progname.h>
#include <common/trace.h>
#include <common/version_stmp.h>
#include <libaegis/change.h>
#include <libaegis/file.h>
#include <libaegis/input/file.h>
#include <libaegis/io.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/project.h>
#include <libaegis/rss.h>
#include <libaegis/rss/feed.h>
#include <libaegis/rss/item.h>
#include <libaegis/xml_node.h>
#include <libaegis/xmltextread/by_node.h>


rss_feed::~rss_feed()
{
    trace(("rss_feed::~rss_feed()\n{\n"));
    project = 0;
    cp = 0;

    while (size())
	delete pop_back();
    delete [] item;
    item = 0;
    item_max = 0;
    trace(("}\n"));
}


rss_feed::rss_feed(project_ty *the_project, change::pointer the_change,
	const nstring &the_file) :
    project(the_project),
    cp(the_change),
    filename(the_file),
    last_build_date(date_string(now())),
    generator(nstring::format("%s %s", progname_get(), version_stamp())),
    docs("http://blogs.law.harvard.edu/tech/rss"),
    item(0),
    item_count(0),
    item_max(0)
{
    trace(("rss_feed::rss_feed()\n"));
}


rss_feed::rss_feed(const rss_feed &arg) :
    project(arg.project),
    cp(arg.cp),
    filename(arg.filename)
{
}


rss_feed &
rss_feed::operator=(const rss_feed &arg)
{
    if (this != &arg)
    {
        project = arg.project;
        cp = arg.cp;
        filename = arg.filename;
    }
    return *this;
}


class xml_node_rss_channel_item:
    public xml_node
{
public:
    xml_node_rss_channel_item(rss_feed *arg) : other(arg) { }
    void element_begin(const nstring &) { other->handle_item(); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item()
{
    trace(("rss_feed::handle_item()\n{\n"));
    push_back(new rss_item());
    trace(("}\n"));
}


rss_item *
rss_feed::back()
    const
{
    assert(item_count > 0);
    return item[item_count - 1];
}


rss_item *
rss_feed::pop_back()
{
    assert(item_count > 0);
    --item_count;
    return item[item_count];
}


class xml_node_rss_channel_item_title:
    public xml_node
{
public:
    xml_node_rss_channel_item_title(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_title(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_title(const nstring &value)
{
    trace(("rss_feed::handle_item_title()\n{\n"));
    back()->handle_title(value);
    trace(("}\n"));
}


class xml_node_rss_channel_item_description:
    public xml_node
{
public:
    xml_node_rss_channel_item_description(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_description(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_description(const nstring &value)
{
    back()->handle_description(value);
}


class xml_node_rss_channel_item_pub_date:
    public xml_node
{
public:
    xml_node_rss_channel_item_pub_date(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_pub_date(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_pub_date(const nstring &value)
{
    back()->handle_pub_date(value);
}


class xml_node_rss_channel_item_link:
    public xml_node
{
public:
    xml_node_rss_channel_item_link(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_link(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_link(const nstring &value)
{
    back()->handle_link(value);
}


class xml_node_rss_channel_item_author:
    public xml_node
{
public:
    xml_node_rss_channel_item_author(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_author(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_author(const nstring &value)
{
    back()->handle_author(value);
}


class xml_node_rss_channel_item_category:
    public xml_node
{
public:
    xml_node_rss_channel_item_category(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_category(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_category(const nstring &value)
{
    back()->handle_category(value);
}


class xml_node_rss_channel_item_comments:
    public xml_node
{
public:
    xml_node_rss_channel_item_comments(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_comments(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_comments(const nstring &value)
{
    back()->handle_comments(value);
}


class xml_node_rss_channel_item_enclosure:
    public xml_node
{
public:
    xml_node_rss_channel_item_enclosure(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_enclosure(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_enclosure(const nstring &value)
{
    back()->handle_enclosure(value);
}


class xml_node_rss_channel_item_guid:
    public xml_node
{
public:
    xml_node_rss_channel_item_guid(rss_feed *arg) :
	other(arg),
	attr(true)
    {
    }

    void
    attribute(const nstring &name, const nstring &value)
    {
	if (name.downcase() == "ispermalink")
	    attr = (value.downcase() == "true");
    }

    void
    text(const nstring &value)
    {
	other->handle_item_guid(value, attr);
    }

private:
    rss_feed *other;
    bool attr;
};


void
rss_feed::handle_item_guid(const nstring &value, bool attr)
{
    back()->handle_guid(value, attr);
}


class xml_node_rss_channel_item_source:
    public xml_node
{
public:
    xml_node_rss_channel_item_source(rss_feed *arg) : other(arg) { }
    void text(const nstring &value) { other->handle_item_source(value); }
private:
    rss_feed *other;
};


void
rss_feed::handle_item_source(const nstring &value)
{
    back()->handle_source(value);
}


void
rss_feed::push_back(rss_item *rip)
{
    if (item_count >= item_max)
    {
	size_t new_item_max = item_max * 2 + 16;
	rss_item **new_item = new rss_item * [new_item_max];
	for (size_t j = 0; j < item_count; ++j)
	    new_item[j] = item[j];
	delete [] item;
	item = new_item;
	item_max = new_item_max;
    }
    item[item_count++] = rip;
}


void
rss_feed::parse()
{
    trace(("rss_feed::parse()\n{\n"));
    user_ty::become scoped(project->get_user());
    if (!os_exists(filename))
    {
	trace(("}\n"));
	return;
    }
    input ip = new input_file(filename);

    //
    // Create the XML file reader, but don't read the file yet.
    //
    xml_text_reader_by_node reader(ip);

    //
    // Register handlers for the various XML elements.
    //
    // It is intentional the none of the <channel> elements are
    // registered, because they are ignored, and the various
    // descriptions of the channel re-written each time.
    //
    xml_node_rss_channel_item i0(this);
    reader.register_node_handler("rss/channel/item", i0);
    xml_node_rss_channel_item_title i1(this);
    reader.register_node_handler("rss/channel/item/title", i1);
    xml_node_rss_channel_item_description i2(this);
    reader.register_node_handler("rss/channel/item/description", i2);
    xml_node_rss_channel_item_pub_date i3(this);
    reader.register_node_handler("rss/channel/item/pubDate", i3);
    xml_node_rss_channel_item_link i4(this);
    reader.register_node_handler("rss/channel/item/link", i4);
    xml_node_rss_channel_item_author i5(this);
    reader.register_node_handler("rss/channel/item/author", i5);
    xml_node_rss_channel_item_category i6(this);
    reader.register_node_handler("rss/channel/item/category", i6);
    xml_node_rss_channel_item_comments i7(this);
    reader.register_node_handler("rss/channel/item/comments", i7);
    xml_node_rss_channel_item_enclosure i8(this);
    reader.register_node_handler("rss/channel/item/enclosure", i8);
    xml_node_rss_channel_item_guid i9(this);
    reader.register_node_handler("rss/channel/item/guid", i9);
    xml_node_rss_channel_item_source i10(this);
    reader.register_node_handler("rss/channel/item/source", i10);

    //
    // Read the XML file, processing registered nodes and ignoring all
    // the rest.
    //
    reader.parse();
    ip.close();
    trace(("}\n"));
}


void
rss_feed::print()
    const
{
    trace(("rss_feed::print()\n{\n"));
    // Open a temp file for writing.
    nstring out_file_name(os_edit_filename(0));
    user_ty::become scoped(project->get_user());
    nstring dir(project_rss_path_get(project, 0));
    if (!os_exists(dir))
        os_mkdir(dir, 0755);
    output::pointer op = output_file::open(out_file_name, false);

    print(op);

    op.reset();

    //
    // Move the temporary output file to the real feed file.
    //
    copy_whole_file(out_file_name.get_ref(), filename.get_ref(), 0);
    os_unlink_errok(out_file_name);
    trace(("}\n"));
}


void
rss_feed::print(output::pointer op)
    const
{
    op->fputs("<?xml version=\"1.0\"?>\n");
    op->fputs("<rss version=\"2.0\">\n");
    op->fputs("<channel>\n");

    if (!title.empty())
	string_write_xml(op, "title", title);
    if (!description.empty())
	string_write_xml(op, "description", description);
    if (!language.empty())
	string_write_xml(op, "language", language);
    if (!link.empty())
	string_write_xml(op, "link", link);
    if (!pub_date.empty())
	string_write_xml(op, "pubDate", pub_date);
    if (!last_build_date.empty())
	string_write_xml(op, "lastBuildDate", last_build_date);
    if (!generator.empty())
	string_write_xml(op, "generator", generator);
    if (!docs.empty())
	string_write_xml(op, "docs", docs);

    // Now emit each item
    for (size_t j = 0; j < item_count; ++j)
    {
	const rss_item *ip = item[j];
	ip->print(op);
    }

    op->fputs("</channel>\n");
    op->fputs("</rss>\n");
}


void
rss_feed::channel_elements_from_project()
{
    trace(("rss_feed::channel_elements_from_project()\n{\n"));
    title = rss_feed_attribute(project, filename, rss_feed_title);
    if (title.empty() && cp)
    {
	title =
	    nstring::format
	    (
		"Changes in state %s",
		cstate_state_ename(cp->cstate_data->state)
	    );
    }
    nstring projname(project_name_get(project));
    title = "Project " + projname + ", " + title;

    //
    // "Phrase or sentence describing the channel."
    //
    // We will set it from the project attributes every time.
    // That way it can be changed.
    //
    description =
	rss_feed_attribute(project, filename, rss_feed_description);
    if (description.empty() && cp)
    {
	cstate_ty *cstate_data = cp->cstate_get();
	description =
	    nstring::format
	    (
		"Feed of changes in state %s",
		cstate_state_ename(cstate_data->state)
	    );
    }

    link = rss_script_name_placeholder;
    link += "/";
    link += nstring(project_name_get(project));
    link += "/?menu";

    language = rss_feed_attribute(project, filename, rss_feed_language);
    if (language.empty())
	language = "en-US";
    trace(("}\n"));
}

void
rss_feed::channel_elements_from_change()
{
    rss_item *rip = new rss_item();
    rip->handle_change(cp);
    push_back(rip);
}


void
rss_feed::title_set(const nstring &arg)
{
    nstring projname(project_name_get(project));
    title = "Project " + projname + ", " + arg.capitalize();
}


void
rss_feed::description_set(const nstring &arg)
{
    description = arg;
}
