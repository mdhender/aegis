//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>
#include <common/ac/stdio.h>
#include <common/ac/pwd.h>

#include <common/debug.h>
#include <common/mem.h>
#include <common/quit.h>
#include <libaegis/adler32.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/quit/action/unlink.h>
#include <libaegis/rfc822/functor/vers_search.h>
#include <libaegis/rfc822/functor/version_prev.h>
#include <libaegis/simpverstool.h>
#include <libaegis/sub.h>


static nstring
build_temp_file_name(const nstring &path)
{
    nstring dirname = os_dirname_relative(path);
    nstring base = os_basename(path);
    return dirname + "/.tmp." + base;
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
    input in = new input_file(input_file_name);

    //
    // Build the header to write to the file.
    //
    rfc822 header(meta_data);
    header.set("content-type", os_magic_file(input_file_name));
    header.set("checksum", calculate_adler32(input_file_name));
    header.set("version", version);
    header.set_off_t("content-length", in->length());
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
        output::pointer os = output_file::open(temp_file_name);
        switch (compression_algorithm)
        {
        case compression_algorithm_none:
            break;

        case compression_algorithm_gzip:
            os = output_filter_gzip::create(os);
            break;

#ifndef DEBUG
        default:
#endif
        case compression_algorithm_unspecified:
        case compression_algorithm_not_set:
            // We will default to the best compression available.
            // For now, that means bzip2

        case compression_algorithm_bzip2:
            os = output_filter_bzip2::create(os);
            break;
        }

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
        header.store(os);
        os << in;

        if (there_are_previous_versions)
        {
            //
            // Add the previous history on the end of the new file.
            // These previous versions should compress *very* well,
            // because thay are usually almost the same as the new
            // version.  Thus the bzip2 compression tables will apply,
            // and provide reasonable compression.
            //
            // Note: gzip resets its tables every 32KB, so it's only
            // good for files smaller than 16KB per version.  The bzip2
            // algorithm resets its tables every 900KB, so it's good
            // for file sizes up to 450KB per version.  Both algorithms
            // keep working beyond these limits, but you don't see the
            // excellent results that are obtain for smaller source
            // files.
            //
            input old_hist_uncom = new input_file(history_file_name);
            // Check both algorithms for decompression, in case we are
            // looking at an older archive.
            input temp = input_gunzip_open(old_hist_uncom);
            input old_history_file = input_bunzip2_open(temp);
            os << old_history_file;
        }
    }

    //
    // Now that we have the new file contents, we can replace the old
    // file with the new file.  (This does the unlink of the old file,
    // as well.)
    //
    os_rename(temp_file_name, history_file_name);
}


// vim: set ts=8 sw=4 et :
