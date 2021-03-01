//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the rss_item class
//

#pragma implementation "rss_item"

#include <ac/string.h>

#include <change.h>
#include <change/branch.h>
#include <cstate.h>
#include <error.h> // for assert
#include <gettime.h>
#include <io.h>
#include <now.h>
#include <output.h>
#include <project.h>
#include <rss.h>
#include <rss/item.h>
#include <user.h>
#include <uuidentifier.h>


rss_item::~rss_item()
{
}


rss_item::rss_item() :
    guidIsPermaLink(true)
{
}


rss_item::rss_item(const rss_item &arg) :
    title(arg.title),
    description(arg.description),
    pub_date(arg.pub_date),
    link(arg.link),
    author(arg.author),
    category(arg.category),
    comments(arg.comments),
    enclosure(arg.enclosure),
    guid(arg.guid),
    guidIsPermaLink(arg.guidIsPermaLink),
    source(arg.source)
{
}


rss_item &
rss_item::operator=(const rss_item &arg)
{
    if (this != &arg)
    {
        title = arg.title;
        description = arg.description;
        pub_date = arg.pub_date;
        link = arg.link;
        author = arg.author;
        category = arg.category;
        comments = arg.comments;
        enclosure = arg.enclosure;
        guid = arg.guid;
	guidIsPermaLink = arg.guidIsPermaLink;
        source = arg.source;
    }
    return *this;
}


void
rss_item::handle_link(const nstring &value)
{
    link = value;
}


void
rss_item::handle_author(const nstring &value)
{
    author = value;
}


void
rss_item::handle_category(const nstring &value)
{
    category = value;
}


void
rss_item::handle_comments(const nstring &value)
{
    comments = value;
}


void
rss_item::handle_enclosure(const nstring &value)
{
    enclosure = value;
}


void
rss_item::handle_guid(const nstring &value, bool isPermaLink)
{
    guid = value;
    guidIsPermaLink = isPermaLink;
}


void
rss_item::handle_source(const nstring &value)
{
    source = value;
}


void
rss_item::handle_title(const nstring &value)
{
    title = value;
}


void
rss_item::handle_description(const nstring &value)
{
    description = value;
}


void
rss_item::handle_pub_date(const nstring &value)
{
    pub_date = value;
}


void
rss_item::print(output_ty *out)
    const
{
    out->fputs("<item>\n");

    if (!title.empty())
    {
	string_write_xml(out, "title", title);
    }

    if (!description.empty())
    {
	string_write_xml(out, "description", description);
    }

    if (!pub_date.empty())
    {
	string_write_xml(out, "pubDate", pub_date);
    }

    if (!link.empty())
    {
        string_write_xml(out, "link", link);
    }

    if (!author.empty())
    {
        string_write_xml(out, "author", author);
    }

    if (!category.empty())
    {
        string_write_xml(out, "category", category);
    }

    if (!comments.empty())
    {
        string_write_xml(out, "comments", comments);
    }

    if (!enclosure.empty())
    {
        string_write_xml(out, "enclosure", enclosure);
    }

    if (!guid.empty())
    {
	out->fputs("<guid");
	if (!guidIsPermaLink)
	    out->fputs(" isPermaLink=\"false\"");
	out->fputc('>');
	out->fputs_xml(guid);
	out->fputs("</guid>\n");
    }

    if (!source.empty())
    {
        string_write_xml(out, "source", source);
    }

    out->fputs("</item>\n");
}


void
rss_item::handle_change(change_ty *cp)
{
    cstate_ty *cstate_data = change_cstate_get(cp);

    //
    // Set the <pubDate> from the most recent change history event.
    //
    cstate_history_list_ty *hlp = cstate_data->history;
    cstate_history_ty *hp = 0;
    assert(hlp);
    assert(hlp->length);
    if (hlp && hlp->length)
	hp = hlp->list[hlp->length - 1];
    assert(hp);
    assert(hp->when);
    if (hp && hp->when)
    {
	time_t when = hp->when;
	handle_pub_date(nstring(date_string(when)));
    }
    else
    {
	time_t when = now();
	handle_pub_date(nstring(date_string(when)));
    }

    //
    // Set the <title> from the change's brief_description.
    //
    handle_title
    (
        nstring::format
        (
            "%s - %s - %s",
            change_version_get(cp)->str_text,
            nstring(cstate_data->brief_description).substring(0, 128).c_str(),
            cstate_state_ename(cstate_data->state)
	)
    );

    //
    // Set the <description> from the change's long description.
    //
    // Look for the first paragraph break (two newlines in a row)
    // for the first 1KB, whichever comes first.
    //
    description = nstring(cstate_data->description);
    const char *para = strstr(description.c_str(), "\n\n");
    size_t desc_len = 1024;
    if (para)
    {
	size_t len = para - description.c_str();
	if (len < desc_len)
    	    desc_len = len;
    }
    description = description.substring(0, desc_len);

    //
    // Set the <author> from the change's history
    //
    user_ty *up =
	(
	    hp && hp->who
	?
	    user_symbolic(cp->pp, hp->who)
	:
	    user_executing(cp->pp)
	);
    author = nstring(user_email_address(up));

    //
    // Create a unique <guid> for every feed item.
    //
    // This is because the <guid> is used to see if the item has been
    // seen before, so if a change appears in the list more than once,
    // once for each state transition, the browser will not ignore the
    // most recent event.
    //
    guid = nstring(universal_unique_identifier());
    guidIsPermaLink = false;

    //
    // Set the <link> from the change's project.
    //
    // Add a link to the item.  The link consists of a script-name
    // placeholder followed by the change number and the ?menu modifier.
    //
    link = rss_script_name_placeholder;
    link +=
	nstring::format
	(
	    "/%s.C%ld/?menu",
	    project_name_get(cp->pp)->str_text,
	    cp->number
	);
}
