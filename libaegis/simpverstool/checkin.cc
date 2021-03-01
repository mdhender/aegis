//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to impliment checkin
//

#include <ac/ctype.h>
#include <ac/stdlib.h>
#include <ac/stdio.h>
#include <ac/pwd.h>

#include <adler32.h>
#include <getpw_cache.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <quit.h>
#include <quit/action/unlink.h>
#include <os.h>
#include <output/file.h>
#include <output/gzip.h>
#include <rfc822/functor/version_prev.h>
#include <rfc822/functor/vers_search.h>
#include <simpverstool.h>
#include <sub.h>


static nstring
build_temp_file_name(const nstring &path)
{
    nstring dirname = os_dirname_relative(path);
    nstring basename = os_basename(path);
    return dirname + "/.tmp." + basename;
}


static nstring
get_user_name()
{
    int uid, gid, umsk;
    os_become_query(&uid, &gid, &umsk);
    struct passwd *pw = getpwuid_cached(uid);
    if (pw)
	return pw->pw_name;
    return nstring::format("uid:%d", uid);
}


void
simple_version_tool::checkin(const nstring &input_file_name)
{
    rfc822 meta_data;
    checkin(input_file_name, meta_data);
}


void
simple_version_tool::checkin(const nstring &input_file_name,
    const rfc822 &meta_data)
{
    rfc822_functor_version_previous prev;
    bool there_are_previous_versions = !list(prev);
    nstring version;
    if (meta_data.is_set("version"))
    {
	version = meta_data.get("version");
	if (version.empty())
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", history_file_name);
	    sc.var_set_string("Number", version);
	    sc.fatal_intl
	    (
		i18n("$filename: version $number invalid")
	    );
	    // NOTREACHED
	}
    }
    else
    {
	if (there_are_previous_versions)
	{
            //
            // If the old version ends with digits, add one to the
            // digits and preserve the prefix.
	    //
	    nstring old_version = prev.get_result();
	    const char *start = old_version.c_str();
	    const char *end = start + old_version.size();
	    const char *cp = end;
	    while (cp > old_version.c_str() && isdigit((unsigned char)cp[-1]))
		--cp;
	    if (cp < end)
	    {
		int n = atoi(cp);
		char buffer[20];
		snprintf(buffer, sizeof(buffer), "%d", n + 1);
		version = nstring(start, cp - start) + nstring(buffer);
	    }
	}
    }
    if (version.empty())
    {
	//
	// When all else fails, start from one.
	//
	version = "1";
    }

    //
    // Make sure the new version number (or whatever it is) will be unique.
    //
    rfc822_functor_version_search snoopy(version);
    list(snoopy);
    if (snoopy.get_result())
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", history_file_name);
	sc.var_set_string("Number", version);
	sc.fatal_intl(i18n("$filename: version $number already exists"));
	// NOTREACHED
    }

    //
    // We have to remember to unlink the temporary file if something
    // goes wrong.  (The temporary file is not in /tmp because we want
    // to do a simple rename at the end, and that has to be on the same
    // file system.)
    //
    nstring temp_file_name = build_temp_file_name(history_file_name);
    quit_register(*new quit_action_unlink(temp_file_name));

    //
    // Open the input file.
    //
    // We will need some information about it to put in the header.
    //
    input_file in(input_file_name);

    //
    // Build the header to write to the file.
    //
    rfc822 header(meta_data);
    header.set("content-type", os_magic_file(input_file_name));
    header.set("checksum", calculate_adler32(input_file_name));
    header.set("version", version);
    header.set("content-length", in.length());
    header.set("content-transfer-encoding", nstring("8bit"));

    //
    // These things are merely defaults in the case where the user does
    // not define any corresponding meta-data herself.
    //
    if (!header.is_set("date"))
	header.set("date", rfc822::date());
    if (!header.is_set("user"))
	header.set("user", get_user_name());

    //
    // We have to scope the output instance, so that the output is all
    // written and closed before we do the rename.
    //
    {
	output_file out_u(temp_file_name);
	output_gzip out(&out_u, false);

	//
        // Each version consists of an RFC 822 header, and the
        // file contents, which can be binary.  The header has a
        // Content-Length entry, so we know exactly how long the content
        // is.  This means we can skip down the file versions when they
        // are simply placed end-to-end.
        //
        // Note that if the source file is plain text, then the
        // (uncompressed) output will be plain text, too.
	//
	header.store(out);
	out << in;

	if (there_are_previous_versions)
	{
	    //
            // Add the previous history on the end of the new file.
            // These previous versions should compress *very* well,
            // because thay are usually almost the same as the new
            // version.  Thus the gzip compression tables will apply,
            // and provide reasonable compression.
	    //
	    input_file old_hist_uncom(history_file_name);
	    input_gunzip old_history_file(&old_hist_uncom, false);
	    out << old_history_file;
	}
    }

    //
    // Now that we have the new file contents, we can replace the old
    // file with the new file.  (This does the unlink of the old file,
    // as well.)
    //
    os_rename(temp_file_name, history_file_name);
}
