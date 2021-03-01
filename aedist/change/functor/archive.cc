//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008, 2009, 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <common/uuidentifier/translate.h>
#include <libaegis/change/branch.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/project.h>

#include <aedist/change/functor/archive.h>


change_functor_archive::~change_functor_archive()
{
}


change_functor_archive::change_functor_archive(bool arg1, project *arg2,
    const char *arg3, const char *arg4, const nstring_list &arg5,
    const nstring_list &arg6) :
    change_functor(arg1),
    exclude_change(arg6),
    fingerprint_suffix(arg4),
    include_change(arg5),
    pp(arg2),
    suffix(arg3)
{
}

void
change_functor_archive::operator()(change::pointer cp)
{
    assert(cp->is_completed());
    if (!cp->is_completed())
        return;

    nstring uuid(cp->uuid_get());
    if (uuid.empty())
        return;

    assert(universal_unique_identifier_valid(uuid));
    if (!universal_unique_identifier_valid(uuid))
        return;

    nstring version = cp->version_get();
    if (!include_change.empty())
    {
        if (include_change.gmatch_candidate(version) <= 0)
            return;
    }
    if (!exclude_change.empty())
    {
        if (exclude_change.gmatch_candidate(version) == 1)
            return;
    }

    //
    // 1) translate the change's UUID in a path name;
    // 2) if the file exists check the fingerprint and, if needed,
    //    force regeneration.
    // 3) if the file does not exists, or the fingerprint mismatch,
    //    create it and calculate the fingerprint;
    //
    nstring uuid_path = uuid_translate(uuid);
    nstring archive_name = uuid_path + suffix;
    trace_nstring(archive_name);
    nstring fp_file_name = uuid_path + fingerprint_suffix;
    trace_nstring(fp_file_name);

    nstring qp(project_name_get(cp->pp));
    bool generate_flag = true;
    os_become_orig();
    bool check = os_exists(fp_file_name) && os_exists(archive_name);
    os_become_undo();

    if (check)
    {
        nstring stored_fp;

        os_become_orig();
        input_file fp_file(fp_file_name);
        fp_file.one_line(stored_fp);
        nstring actual_fp(os_fingerprint(archive_name.get_ref()));
        os_become_undo();
        generate_flag = (stored_fp != actual_fp);
    }

    if (generate_flag)
    {
        nstring cmd =
            nstring::format
            (
                "aedist -send -p %s -change %ld -output %s "
                "-ndh -nmh -cte=none -comp-alg=gzip",
                qp.quote_shell().c_str(),
                cp->number,
                archive_name.c_str()
            );

        os_become_orig();
        nstring curdir(os_curdir());
        os_unlink_errok(fp_file_name);
        os_unlink_errok(archive_name);
        os_mkdir_between(curdir, archive_name, 0755);

        os_execute(cmd, OS_EXEC_FLAG_SILENT, curdir);

        nstring current_fp(os_fingerprint(archive_name.get_ref()));
        output::pointer os = output_file::open(fp_file_name);
        os->fputs(current_fp);
        os->end_of_line();
        os.reset();
        os_become_undo();
    }
}


// vim: set ts=8 sw=4 et :
