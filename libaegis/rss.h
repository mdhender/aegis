//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee
//      Copyright (C) 2007, 2008 Peter Miller
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

#ifndef LIBAEGIS_RSS_H
#define LIBAEGIS_RSS_H

#include <common/nstring.h>

struct project_ty; // forward


/**
  * Possible RSS-related attributes that can be pulled from the project-specific
  * attributes.
  */
enum rss_feed_attribute
{
    rss_feed_description,
    rss_feed_title,
    rss_feed_language,
};


/**
  * Placeholder used in feed files.  When the feed is served up by aeget,
  * the placeholder is replaced with the http script name.
  */
extern const nstring rss_script_name_placeholder;


/**
  * Find the filename for an RSS feed.
  *
  * This function searches the project-specific attributes for entries of
  * the form: <pre> rss:feedfilename-<filename> = "<state>"; </pre>
  * When an entry with a matching state is found, the filename is returned.
  * Available states are:
  * <pre>
  * awaiting_development
  * being_developed
  * awaiting_review
  * being_reviewed
  * awaiting_integration
  * being_integrated
  * completed
  * </pre>
  *
  * \param pp
  *   A pointer to the project to use.  This is used to find the
  *   project-specific attributes.
  *
  * \param state
  *   A string containing one of the above states.
  *
  * \return
  *   The full path to the filename associated with that state.  The
  *   empty string if no filename has been associated with the specified
  *   state (this includes illegal state strings).
  */
nstring rss_feed_filename(project_ty *pp, const nstring &state);

/**
  * Find the RSS Channel attribute for an RSS feed file.
  *
  * This function searches the project-specific attributes for an entry of the
  * form: <pre> rss:feed<attribute>-<filename> = "<value>"; </pre>
  * The value corresponding to the specified filename is returned.
  * If an appropriate entry is not found, 0 is returned.
  *
  * \param pp
  *   A pointer to the project to use.  This is used to find the
  *   project-specific attributes.
  *
  * \param filename
  *   The RSS feed filename for which the attribute is to be located.
  *
  * \param attribute
  *   The attribute to look for.
  *
  * \return
  *   The attribute's value, or the empty string if not found.
  */
nstring rss_feed_attribute(project_ty *pp, const nstring &filename,
    rss_feed_attribute attribute);

/**
  * Get a URL for the specified project.
  *
  * The returned URL contains a URL placeholder and includes scriptname
  * and project name.
  */
nstring rss_get_project_url(project_ty *pp);

/**
  * Look for an RSS feed file based on change state.  Add an RSS item if a feed
  * file is found.  This function should be called by each of the
  * ae<change-state> commands.
  *
  * \param pp
  *   The project details.
  *
  * \param cp
  *   The changeset details.
  */
void rss_add_item_by_change(project_ty *pp, change::pointer cp);

/**
  * Add an RSS item to the specified RSS feed file.
  *
  * \param filename
  *   The filename to add the new item to.
  *
  * \param pp
  *   The project involved.
  *
  * \param cp
  *   The changeset to which the new item relates.
  */
void rss_add_item(const nstring &filename, project_ty *pp, change::pointer cp);

#endif // LIBAEGIS_RSS_H
