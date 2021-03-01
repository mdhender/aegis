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
// MANIFEST: implementation of the get_rss class
//

#include <ac/stdio.h>
#include <ac/string.h>
#include <emit/project.h>
#include <get/rss.h>
#include <gif.h>
#include <http.h>
#include <input/file.h>
#include <libdir.h>
#include <nstring.h>
#include <os.h>
#include <output/file.h>
#include <project.h>
#include <rss.h>
#include <str_list.h>


static void
whinge(project_ty *pp, const nstring &filename)
{
    html_header(pp, 0);

    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(",\nPuzzlement</title></head><body>\n");
    html_header_ps(pp, 0);
    printf("<h1 align=center>");
    emit_project(pp);
    printf(",<br>\nPuzzlement</h1>\n");
    printf("Cannot find file %s\n", filename.c_str());

    html_footer(pp, 0);
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
    if (modifier->nstrings >= 1)
    {
        // Read the filename from the second modifier.
        nstring rss_filename(modifier->string[0]->str_text);

        // Figure out the full path to the RSS file and check it exists.
        nstring path(project_rss_path_get(pp, 1));
        path += "/";
        path += rss_filename;
        project_become(pp);
        bool file_exists = os_exists(path);
	if (file_exists)
        {
            // Read in the file.  Replace the script name placeholders with
            // real script names and write to stdout.
            input_ty *ip = input_file_open(path.get_ref());
            if (ip != 0)
            {
                printf("Content-Type: application/rss+xml\n\n");

                nstring line;
                while (ip->one_line(line))
                {
		    // This line may contain the placeholder.
		    // Replace it with the real script name.
		    // There may be more than one.
		    const char *pos = line.c_str();
		    for (;;)
		    {
			const char *location =
			    strstr
			    (
				pos,
				rss_script_name_placeholder.c_str()
			    );
			if (0 == location)
			{
			    // The rest of the line does not contain the
			    // placeholder.  Just dump it as-is.
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
        }
        project_become_undo();

        if (!file_exists)
        {
            whinge(pp, rss_filename);
        }
    }
}
