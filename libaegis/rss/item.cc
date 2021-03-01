//
// aegis - project change supervisor
// Copyright (C) 2005 Matthew Lee
// Copyright (C) 2007-2009, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <common/gettime.h>
#include <common/now.h>
#include <common/uuidentifier.h>
#include <libaegis/change/branch.h>
#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/io.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/rss.h>
#include <libaegis/rss/item.h>
#include <libaegis/user.h>


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
rss_item::print(output::pointer out)
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
rss_item::handle_change(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();

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
    // See description comment, below, about double encoding.
    //
    // FIXME: the amount of text (that 128 you see below) should be
    // configurable.
    //
    nstring tmp =
        nstring::format
        (
            "%s - %s - %s",
            cp->version_get().c_str(),
            cp->brief_description_get().substr(0, 128).c_str(),
            cstate_state_ename(cstate_data->state)
        );
    title = tmp.html_quote();

    //
    // Set the <description> from the change's long description.
    //
    // This is made slightly ugly by how the browser is expected to turn
    // the RSS XML into HTML.  In Mozilla Firefox, for example, the text
    // is glued into a table cell without any interpretation.  This
    // means it must be encoded twice: once for the HTML, and a second
    // time for the XML carrier.
    //
    // FIXME: the amount of text (that 2000 you see below) should be
    // configurable.
    //
    description =
        nstring(cstate_data->description).substr(0, 2000).html_quote(true);

    //
    // Set the <author> from the change's history
    //
    user_ty::pointer up =
        (hp && hp->who ? user_ty::create(nstring(hp->who)) : user_ty::create());
    author = up->get_email_address();

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
            project_name_get(cp->pp).c_str(),
            cp->number
        );
}


// vim: set ts=8 sw=4 et :
