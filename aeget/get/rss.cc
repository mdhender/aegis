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

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/libdir.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/change/branch.h>
#include <libaegis/emit/project.h>
#include <libaegis/gif.h>
#include <libaegis/http.h>
#include <libaegis/input/file.h>
#include <libaegis/input/string.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/memory.h>
#include <libaegis/project.h>
#include <libaegis/rss.h>
#include <libaegis/rss/feed.h>

#include <aeget/get/rss.h>


static void
postprocess(input &ip)
{
    printf("Content-Type: application/rss+xml\n\n");
    nstring line;
    while (ip->one_line(line))
    {
	//
        // This line may contain the placeholder.  Replace it with the
        // real script name.  There may be more than one.
	//
	const char *pos = line.c_str();
	for (;;)
	{
	    const char *location =
		strstr(pos, rss_script_name_placeholder.c_str());
	    if (0 == location)
	    {
                //
                // The rest of the line does not contain the
                // placeholder.  Just dump it as-is.
		//
		printf("%s\n", pos);
		break;
	    }
	    printf("%.*s", (int)(location - pos), pos);

	    // Swap the placeholder for the host/scriptname
	    printf("%s", http_script_name());

	    // Move past the placeholder
	    pos = location + rss_script_name_placeholder.size();
	}
    }
}


void
get_rss(project_ty *pp, string_ty *, string_list_ty *modifier)
{
    //
    // "Get RSS" requests should have the following modifiers:
    //  1. rss
    //  2. filename
    //
    // The file needs to be read from disk, parsed to replace script name
    // placeholders with real script names and dumped to stdout.
    //
    change::pointer pcp = pp->change_get();
    if (change_is_a_branch(pcp) && modifier->nstrings >= 1)
    {
        // Read the filename from the second modifier.
        nstring rss_filename(modifier->string[0]->str_text);

        // Figure out the full path to the RSS file and check it exists.
        nstring path(project_rss_path_get(pp, 1));
        path += "/";
        path += rss_filename;
        user_ty::become scoped(pp->get_user());
        bool file_exists = os_exists(path);
	if (file_exists)
        {
            // Read in the file.  Replace the script name placeholders with
            // real script names and write to stdout.
            input ip = input_file_open(path.get_ref());
	    assert(ip.is_open());
            if (ip.is_open())
            {
		postprocess(ip);
		ip.close();
		return;
            }
        }
    }

    //
    // Build a fake feed, and set its description to say what they did wrong.
    // We write it to a string, so that we can turn it into anf input,
    // so that the feed can be post-processed to insert the feed script
    // name.
    //
    fprintf(stderr, "%s: %d\n", __FILE__, __LINE__);
    rss_feed fake(pp, 0, "");
    fprintf(stderr, "%s: %d\n", __FILE__, __LINE__);
    fake.channel_elements_from_project();
    if (modifier->nstrings >= 1)
	fake.title_set(nstring(modifier->string[0]));
    else
	fake.title_set("No Feed Name");
    if (!change_is_a_branch(pcp))
	fake.description_set("Not an active branch.");
    else if (modifier->nstrings >= 1)
	fake.description_set("No feed of this name.");
    else
	fake.description_set("No feed name specified.");
    output_memory::mpointer op = output_memory::create();
    fake.print(op);
    input ip = new input_string(op->mkstr());
    postprocess(ip);
}
