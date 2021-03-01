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
// MANIFEST: implementation of the rss class
//

#include <ac/string.h>

#include <change.h>
#include <pconf.h>
#include <project.h>
#include <rss.h>
#include <rss/feed.h>
#include <rss/item.h>
#include <trace.h>


const nstring rss_script_name_placeholder("@@SCRIPTNAME@@");


nstring
rss_feed_filename(project_ty *pp, const nstring &state)
{
    // Get the project-specific attributes.
    pconf_ty *pconf_data = project_pconf_get(pp);
    if (pconf_data->project_specific != 0)
    {
        // We are only interested in those with the correct prefix.
        const nstring feed_filename_prefix = "rss:feedfilename-";
        for
        (
            size_t index = 0;
            index < pconf_data->project_specific->length;
            ++index
        )
        {
            attributes_ty *ap = pconf_data->project_specific->list[index];
            if
            (
                ap->name != 0
            &&
                ap->name->str_length > feed_filename_prefix.size()
            &&
                (
                    0
                ==
                    strncasecmp
                    (
                        ap->name->str_text,
                        feed_filename_prefix.c_str(),
                        feed_filename_prefix.size()
                    )
                )
            )
            {
                // Does the project-specific attribute we've found contain
                // the correct state?
                if (0 != strstr(ap->value->str_text, state.c_str()))
                {
                    // We've found an entry with the right prefix and state.
                    // What's the associated filename?  Return the full path.
                    nstring path = project_rss_path_get(pp, 1);
                    path += "/";
                    path += ap->name->str_text + feed_filename_prefix.size();
                    return path;
                }
            }
        }
    }
    return "";
}


static const char *
rss_feed_attr_to_str(enum rss_feed_attribute attribute)
{
    switch (attribute)
    {
    case rss_feed_description:
        return "description";

    case rss_feed_title:
        return "title";

    case rss_feed_language:
        return "language";
    }

    //
    // By having this default outside the switch, it gives GCC the
    // chance to tell us we forgot an enum.
    //
    return "unknown";
}


nstring
rss_feed_attribute(project_ty *pp, const nstring &filename,
    enum rss_feed_attribute attribute)
{
    // Get the project-specific attributes.
    pconf_ty *pconf_data = project_pconf_get(pp);
    if (pconf_data->project_specific != 0)
    {
        // We are only interested in those with the correct prefix.
        nstring feed_attr_prefix("rss:feed");
        feed_attr_prefix += rss_feed_attr_to_str(attribute);
        feed_attr_prefix += "-";

        nstring feed_attr = feed_attr_prefix + filename;

        // Search through each project-specific attribute.
        for
        (
            size_t index = 0;
            index < pconf_data->project_specific->length;
            ++index
        )
        {
            attributes_ty *ap = pconf_data->project_specific->list[index];
            if
            (
                (ap->name != 0)
            &&
                (ap->name->str_length > feed_attr_prefix.size())
            &&
                (
                    0
                ==
                    strncasecmp
                    (
                        ap->name->str_text,
                        feed_attr_prefix.c_str(),
                        feed_attr_prefix.size()
                    )
                )
            )
            {
                return nstring(ap->value);
            }
        }
    }
    return "";
}


nstring
rss_get_project_url(project_ty *pp)
{
    nstring url = rss_script_name_placeholder;
    url += "/";
    url += nstring(project_name_get(pp));
    return url;
}


void
rss_add_item_by_change(project_ty *pp, change_ty *cp)
{
    trace(("rss_add_item_by_change()\n{\n"));
    cstate_ty *cstate_data = change_cstate_get(cp);
    nstring feed_filename =
        rss_feed_filename(pp, cstate_state_ename(cstate_data->state));
    if (!feed_filename.empty())
    {
        rss_add_item(feed_filename, pp, cp);
    }
    trace(("}\n"));
}


void
rss_add_item(const nstring &filename, project_ty *pp, change_ty *cp)
{
    trace(("rss_add_item()\n{\n"));
    cstate_ty *cstate_data = change_cstate_get(cp);
    if (!cstate_data->brief_description)
            cstate_data->brief_description = str_from_c("none");
    if (!cstate_data->description)
            cstate_data->description = str_from_c("none");
    rss_feed feed(pp, cp, filename);
    feed.channel_elements_from_project();

    //
    // Add a feed item based on the given change.
    //
    feed.channel_elements_from_change();

    //
    // Append the existing contents of the file.
    // Make sure we don't ever have too many elements.
    // Write the file back out.
    //
    feed.parse();
    while (feed.size() > 10)
	delete feed.pop_back();
    feed.print();
    trace(("}\n"));
}
