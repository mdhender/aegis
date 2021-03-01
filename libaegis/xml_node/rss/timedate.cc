//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee
//      Copyright (C) 2008 Peter Miller
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

#include <common/gettime.h>
#include <common/now.h>
#include <libaegis/output.h>
#include <libaegis/xml_node/rss/timedate.h>
#include <libaegis/xmltextread/by_node.h>


xml_node_rss_timedate::~xml_node_rss_timedate()
{
}


xml_node_rss_timedate::xml_node_rss_timedate(xml_text_reader_by_node &reader,
        output::pointer out, bool use_current_time) :
    xml_node_rss(reader, out),
    set_current_time(use_current_time)
{
    if (use_current_time)
    {
        reader.register_node_handler("rss/channel/lastBuildDate", *this);
    }
    else
    {
        reader.register_node_handler("rss/channel/pubDate", *this);
    }

    // The timedate handler will always be called after some of the parent
    // classes handlers -- that's dictated by the structure of RSS feed files.
    // So we can safely set xml_version_printed to true, so we don't go
    // printing it again prior to any datetime elements.
    xml_version_printed = true;
}


xml_node_rss_timedate::xml_node_rss_timedate(
    const xml_node_rss_timedate &arg) :
    xml_node_rss(arg),
    set_current_time(arg.set_current_time)
{
    xml_version_printed = true;
}


void
xml_node_rss_timedate::text(const nstring &value)
{
    terminate_element_begin();

    if (value.empty())
    {
        // We don't care what the value of the date/time string is.
        // Just write out either the current date/time or just after
        // midnight on the current date.
	time_t time_string = now();
	if (!set_current_time)
	{
	    time_t early_this_morning =
		time_string - (time_t)(time_string % 86400) + 1;
	    time_string = early_this_morning;
	}
	deeper->fputs(date_string(time_string));
    }
    else
	deeper->fputs_xml(value);
}
