//
//      aegis - project change supervisor
//      Copyright (C) 1991-1995, 1998, 1999, 2001-2006, 2008, 2012 Peter Miller
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
#include <common/ac/ctype.h>
#include <common/ac/errno.h>

#include <common/error.h>
#include <libaegis/file.h>
#include <libaegis/fopen_nfs.h>
#include <libaegis/glue.h>
#include <libaegis/input/file_text.h>
#include <common/mem.h>
#include <common/nstring/accumulator.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>


//
// NAME
//      copy_whole_file
//
// SYNOPSIS
//      void copy_whole_file(string_ty *from, string_ty *to, int cmt);
//
// DESCRIPTION
//      The copy_whole_file function is used to
//      copy a file from one place to another.
//
//      The time-last-modified is preserved, but not the permissions.
//      Permissions will be 0666 and modified by the active umask.
//
// ARGUMENTS
//      from    - source path
//      to      - destination path
//      cmt     - copy modify time of file?
//
// CAVEAT
//      Assumes that the user has already been set,
//      uid, gid and umask.
//

void
copy_whole_file(string_ty *from, string_ty *to, int cmt)
{
    trace(("copy_whole_file(from = %p, to = %p)\n{\n",
        from, to));
    os_become_must_be_active();
    trace_string(from->str_text);
    trace_string(to->str_text);
    if (to->str_length)
    {
        if (glue_copyfile(from->str_text, to->str_text))
        {
            sub_context_ty  *scp;
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name1", from);
            sub_var_set_string(scp, "File_Name2", to);
            fatal_intl(scp, i18n("cp $filename1 $filename2: $errno"));
            // NOTREACHED
        }
    }
    else
    {
        if (glue_catfile(from->str_text))
        {
            sub_context_ty  *scp;
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", from);
            fatal_intl(scp, i18n("cat $filename: $errno"));
            // NOTREACHED
        }
    }

    //
    // copy last-time-modified of the file
    //
    if (cmt && to->str_length)
    {
        os_mtime_set(to, os_mtime_actual(from));
    }
    trace(("}\n"));
}

void
copy_whole_file(const nstring &from, const nstring &to, bool cmt)
{
    copy_whole_file(from.get_ref(), to.get_ref(), cmt);
}


string_ty *
read_whole_file(string_ty *fn)
{
    nstring result(read_whole_file(nstring(fn)));
    return str_copy(result.get_ref());
}


nstring
read_whole_file(const nstring &fn)
{
    nstring_accumulator acc;
    os_become_must_be_active();
    input fp = input_file_text_open(fn.get_ref());
    for (;;)
    {
        int c = fp->getch();
        if (c == EOF)
            break;
        acc.push_back(c);
    }
    while (acc.size() > 0 && isspace((unsigned char)acc.back()))
        acc.pop_back();
    return acc.mkstr();
}


bool
files_are_different(const nstring &s1, const nstring &s2)
{
    os_become_must_be_active();
    int result = glue_file_compare(s1.c_str(), s2.c_str());
    if (result < 0)
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name1", s1);
        sc.var_set_string("File_Name2", s2);
        sc.fatal_intl(i18n("cmp $filename1 $filename2: $errno"));
        // NOTREACHED
    }
    return (result != 0);
}


int
files_are_different(string_ty *s1, string_ty *s2)
{
    return files_are_different(nstring(s1), nstring(s2));
}


// vim: set ts=8 sw=4 et :
