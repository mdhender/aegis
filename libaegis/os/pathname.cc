//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2006, 2008, 2012 Peter Miller
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
// This file, in selected areas, DOES NOT use the glue interface.
// This is intentional - there can be no security breach, and no
// distortion of semantics.
//

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>
#include <common/ac/unistd.h>
#include <common/ac/mntent.h>
#include <common/ac/sys/clu.h>

#include <common/mem.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/stracc.h>
#include <common/trace.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


//
// NAME
//      os_curdir_sub - get current directory
//
// SYNOPSIS
//      string_ty *os_curdir_sub(void);
//
// DESCRIPTION
//      The os_curdir_sub function is used to determine the system's idea
//      of the current directory.
//
// RETURNS
//      A pointer to a string in dynamic memory is returned.
//      A null pointer is returned on error.
//
// CAVEAT
//      DO NOT use str_free() on the value returned.
//

static string_ty *
os_curdir_sub(void)
{
    static string_ty *s;

    os_become_must_be_active();
    if (!s)
    {
        char            buffer[2000];
        char            *pwd;

        pwd = getenv("PWD");
        if (pwd && *pwd == '/')
        {
            struct stat     st1;
            struct stat     st2;

            if
            (
                glue_stat(".", &st1) == 0
            &&
                glue_stat(pwd, &st2) == 0
            &&
                st1.st_dev == st2.st_dev
            &&
                st1.st_ino == st2.st_ino
            )
            {
                s = str_from_c(pwd);
                return s;
            }
        }

        if (!glue_getcwd(buffer, sizeof(buffer)))
        {
            sub_context_ty  *scp;
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            fatal_intl(scp, i18n("getcwd: $errno"));
            // NOTREACHED
        }
        assert(buffer[0] == '/');
        s = str_from_c(buffer);
    }
    return s;
}


//
// NAME
//      os_curdir - full current directory path
//
// SYNOPSIS
//      string_ty *os_curdir(void);
//
// DESCRIPTION
//      Os_curdir is used to determine the pathname
//      of the current directory.  Automounter vaguaries will be elided.
//
// RETURNS
//      A pointer to a string in dynamic memory is returned.
//      A null pointer is returned on error.
//
// CAVEAT
//      Use str_free() when you are done with the value returned.
//

string_ty *
os_curdir(void)
{
    static string_ty *result;

    os_become_must_be_active();
    if (!result)
    {
        string_ty       *dot;

        dot = str_from_c(".");
        result = os_pathname(dot, 1);
        str_free(dot);
    }
    return str_copy(result);
}


//
// NAME
//      has_prefix
//
// SYNOPSIS
//      string_ty *has_prefix(string_ty *prefix, string_ty *candidate);
//
// DESCRIPTION
//      The has_prefix function is used to test if the "prefix" string
//      is a prefix of the "candidate" string.
//
// RETURNS
//      On success: the remainder of the string with the prefix removed.
//      On failure: NULL.
//

static string_ty *
has_prefix(string_ty *pfx, string_ty *path)
{
    if (str_equal(pfx, path))
        return str_copy(path);
    if
    (
        pfx->str_length < path->str_length
    &&
        path->str_text[pfx->str_length] == '/'
    &&
        0 == memcmp(pfx->str_text, path->str_text, pfx->str_length)
    )
    {
        return
            str_n_from_c
            (
                path->str_text + pfx->str_length,
                path->str_length - pfx->str_length
            );
    }
    return 0;
}


//
// NAME
//      has_a_prefix
//
// SYNOPSIS
//      string_ty *has-a_prefix(string_list_ty *prefix, string_ty *candidate);
//
// DESCRIPTION
//      The has_a_prefix function is used to test if any one of the
//      "prefix" strings is a prefix of the "candidate" string.
//
// RETURNS
//      On success: the remainder of the string with the prefix removed.
//      On failure: NULL.
//

static string_ty *
has_a_prefix(string_list_ty *pfx, string_ty *path)
{
    size_t          j;
    string_ty       *result;

    for (j = 0; j < pfx->nstrings; ++j)
    {
        result = has_prefix(pfx->string[j], path);
        if (result)
            return result;
    }
    return 0;
}


//
// NAME
//      get_prefix_list
//
// SYNOPSIS
//      string_list_ty *get_prefix_list(void);
//
// DESCRIPTION
//      The get_prefix_list function is used to get the list of possible
//      auto mount points from the AEGIS_AUTOMOUNT_POINTS environment
//      variable.
//
//      The value is colon separated.  values not starting with slash,
//      and the root directory, are silently ignored.
//
// RETURNS
//      string list of prefixes
//      DO NOT delete it *ever* because it is cached.
//

static string_list_ty *
get_prefix_list(void)
{
    static string_list_ty *prefix;
    const char      *cp;
    string_ty       *s;
    size_t          j;

    if (prefix)
        return prefix;

    //
    // Pull the value out of the relevant environment
    // variable, and break it into pieces (it's colon
    // separated).
    //
    prefix = new string_list_ty();
    cp = getenv("AEGIS_AUTOMOUNT_POINTS");
    if (!cp)
        cp = "/tmp_mnt:/a:/.automount";
    s = str_from_c(cp);
    string_list_ty tmp;
    tmp.split(s, ":");
    str_free(s);

    //
    // Rip off any trailing slashes.
    //
    for (j = 0; j < tmp.nstrings; ++j)
    {
        s = tmp.string[j];
        if (s->str_text[0] != '/')
            continue;
        size_t len = s->str_length;
        while (len > 0 && s->str_text[len - 1] == '/')
            --len;
        if (len != s->str_length)
        {
            if (len > 0)
            {
                s = str_n_from_c(s->str_text, len);
                prefix->push_back_unique(s);
                str_free(s);
            }
        }
        else
            prefix->push_back_unique(s);
    }
    return prefix;
}


//
// NAME
//      get_auto_mount_dirs
//
// SYNOPSIS
//      string_list_ty *get_auto_mount_dirs(void);
//
// DESCRIPTION
//      The get_auto_mount_dirs function is used to grab all the active
//      mount points with an automount prefix.
//
//      Because we are called (indirectly) by os_pathname, all of
//      the relevant auto mount activities have taken place, and thus
//      the mount table (obtained through the getmntent api) will be
//      up-to-date, and contain mount entries with auto mount point prefixes.
//
//      We cache, and re-read if this the MOUNTED file changes.
//
// RETURNS
//      String list of actual automounted mount points.
//      DO NOT delete it *ever* because it is cached.
//

static string_list_ty *
get_auto_mount_dirs(string_list_ty *prefix)
{
    FILE            *fp;
    static string_list_ty *dirs;
    static struct stat mntent_st;
    static string_ty *slash;
    int             err;
    struct stat     st;
    string_ty       *p1;
    struct stat     st1;
    string_ty       *p2;
    struct stat     st2;

    fp = setmntent(MOUNTED, "r");
    if (!fp)
    {
        if (dirs)
            dirs->clear();
        return dirs;
    }

    //
    // The "fp" variable is probably not a real FILE*, so don't
    // use it like one!  E.g. Cygwin cast it to FILE*, but there's
    // actually a whole 'nother data structure behind there, and it
    // GPFs if you try to access it as if it were a FILE*.
    //
    err = stat(MOUNTED, &st);
    if (err)
        memset(&st, 0, sizeof(st));
    if (dirs && mntent_st.st_mtime == st.st_mtime)
    {
        endmntent(fp);
        return dirs;
    }
    mntent_st = st;

    if (!slash)
        slash = str_from_c("/");
    if (dirs)
        dirs->clear();
    else
        dirs = new string_list_ty();
    for (;;)
    {
        struct mntent   *mep;
        string_ty       *dir;
        string_ty       *tmp;

        mep = getmntent(fp);
        if (!mep)
            break;
        dir = str_from_c(mep->mnt_dir);

        //
        // Ignore "/" because everything is below it.
        //
        if (str_equal(dir, slash))
        {
          the_next_one:
            str_free(dir);
            continue;
        }

        //
        // If the mount point doesn't have an automount
        // point prefix, skip this mount point.
        //
        // We are called by os_pathname, which has just
        // exersized all of the symbolic links and automount
        // thingies.  So, they will all be in the mount table.
        // (Except for really weird cases of symlinks between
        // NFS file systems, with some servers fast and some
        // really really slow, but don't worry about that.)
        //
        tmp = has_a_prefix(prefix, dir);
        if (!tmp)
            goto the_next_one;

        //
        // Simply fiddling with the path is a security
        // hole.  We must make sure that the auto-mounted
        // and un-auto-mounted paths give the same answer.
        //
        p1 = str_format("%s/.", dir->str_text);
        err = lstat(p1->str_text, &st1);
        str_free(p1);
        if (err)
            goto the_next_one;
        p2 = str_format("%s/.", tmp->str_text);
        str_free(tmp);
        err = lstat(p2->str_text, &st2);
        str_free(p2);
        if (err)
            goto the_next_one;
        if (st1.st_ino != st2.st_ino || st1.st_dev != st2.st_dev) //lint !e81
            goto the_next_one;

        //
        // Everything checks out,
        // remember this one.
        //
        dirs->push_back(dir);
        str_free(dir);
    }
    endmntent(fp);
    return dirs;
}


//
// NAME
//      remove_automounter_prefix
//
// SYNOPSIS
//      string_ty *remove_automounter_prefix(string_ty *path);
//
// DESCRIPTION
//      The remove_automounter_prefix function is used to remove
//      any automounter prefix that may be present on an absolute
//      path name.  The prefixes to check for are obtained from the
//      AEGIS_AUTOMOUNT_POINTS environment variable.
//
// RETURNS
//      string_ty * - pointer dynamically allocated string.  Use str_free
//      when done with it.
//
// CAVEAT
//      This function is dangerous.  Use with extreme care.
//

static string_ty *
remove_automounter_prefix(string_ty *path)
{
    string_list_ty *prefix;
    string_list_ty *amdl;
    string_ty      *result;

    //
    // Get the list of possible automount prefixes.
    //
    prefix = get_prefix_list();

    //
    // Get the list of automounted mount points.
    // It's cached, so it usually doesn't take long.
    //
    amdl = get_auto_mount_dirs(prefix);

    //
    // Look for leading path prefixes.
    //
    result = has_a_prefix(prefix, path);
    if (result)
    {
        string_ty       *tmp;

        //
        // Now see if the path (known to have an auto mount
        // prefix) is below an automounted mount point (also
        // known to have an auto mount prefix).
        //
        tmp = has_a_prefix(amdl, path);
        if (tmp)
        {
            str_free(tmp);
            return result;
        }

        //
        // The path is below an auto mount directory, but
        // not in the mount table, so it's bogus in some way.
        // Ignore it, in case it is an attempt to subvert Aegis
        // into a security breach.
        //
        str_free(result);
    }

    //
    // No match, return the original.
    //
    return str_copy(path);
}


//
// NAME
//      memb
//
// SYNOPSIS
//      string_ty *memb(void);
//
// DESCRIPTION
//      The memb function is used to determine the member name of an
//      OSF/1 cluster member name.
//
// CAVEAT
//      This is only meaningful on OSF/1
//

#ifdef HAVE_CLU_INFO

static string_ty *
memb(void)
{
    static string_ty *result;
    if (!result)
    {
        char            name[MAXHOSTNAMELEN];
        memberid_t      my_id;

        if
        (
            clu_info(CLU_INFO_MY_ID, &my_id) < 0
        ||
            clu_info(CLU_INFO_NODENAME_BY_ID, my_id, name, sizeof(name)) < 0
        )
            result = str_from_c("member0");
        else
            result = str_from_c(name);
    }
    return result;
}


//
// NAME
//      magic_memb_replace
//
// SYNOPSIS
//      string_ty *magic_memb_replace(string_ty *);
//
// DESCRIPTION
//      The magic_memb_replace function is used to replace instances
//      of "{memb}" in a symbolic link's value with the name of the
//      cluster member.
//
// CAVEAT
//      This is only meaningful on OSF/1
//

static string_ty *
magic_memb_replace(string_ty *s)
{
    static stracc_t sa;
    char            *cp;
    char            *end;
    char            *ep;

    sa.clear();
    cp = s->str_text;
    end = s->str_text + s->str_length;
    while (cp < end)
    {
        ep = memchr(cp, '{', end - cp);
        if (!ep)
        {
            stracc_chars(&sa, cp, end - cp);
            break;
        }
        if (ep > cp)
        {
            stracc_chars(&sa, cp, ep - cp);
            cp = ep;
        }
        if (cp + 6 <= end && 0 == memcmp(cp, "{memb}", 6))
        {
            string_ty       *name;

            name = memb();
            stracc_chars(&sa, name->str_text, name->str_length);
            cp += 6;
        }
        else
            sa.push_back(*cp++);
    }
    return sa.mkstr();
}

#endif


//
// NAME
//      os_pathname - determine full file name
//
// SYNOPSIS
//      string_ty *os_pathname(string_ty *path, int resolve);
//
// DESCRIPTION
//      Os_pathname is used to determine the full path name
//      of a partial path given.
//
// ARGUMENTS
//      path    - path to canonicalize
//      resolve - non-zero if should resolve symlinks, 0 if not
//
// RETURNS
//      pointer to dynamically allocated string.
//
// CAVEAT
//      Use str_free() when you are done with the value returned.
//

string_ty *
os_pathname(string_ty *path, int resolve)
{
    int             found;
    string_ty       *result;

    //
    // Change relative pathnames to absolute
    //
    trace(("os_pathname(path = %p)\n{\n", path));
    if (!path)
        path = os_curdir();
    if (resolve)
        os_become_must_be_active();
    trace_string(path->str_text);
    if (path->str_text[0] != '/')
        path = os_path_join(os_curdir_sub(), path);
    else
        path = str_copy(path);

    //
    // Take kinks out of the pathname
    //
    static stracc_t ac;
    ac.clear();
    size_t ipos = 0;
    found = 0;
#ifdef S_IFLNK
    int loop = 0;
#endif
    while (!found)
    {
        //
        // get the next character
        //
        unsigned char c = path->str_text[ipos];
        if (c)
            ipos++;
        else
        {
            found = 1;
            c = '/';
        }

        //
        // remember the normal characters
        // until get to slash
        //
        if (c != '/')
        {
            ac.push_back(c);
            continue;
        }

        //
        // leave root alone
        //
        if (ac.empty())
        {
            ac.push_back(c);
            continue;
        }

        //
        // "/.." -> "/"
        //
        if (ac.size() == 3 && ac[1] == '.' && ac[2] == '.')
        {
            ac.pop_back();
            ac.pop_back();
            continue;
        }

        //
        // "a//" -> "a/"
        //
        if (ac.back() == '/')
            continue;

        //
        // "a/./" -> "a/"
        //
        if (ac.size() >= 2 && ac.back() == '.' && ac[ac.size() - 2] == '/')
        {
            ac.pop_back();
            continue;
        }

        //
        // "a/b/../" -> "a/"
        //
        if
        (
            ac.size() > 3
        &&
            ac[ac.size() - 1] == '.'
        &&
            ac[ac.size() - 2] == '.'
        &&
            ac[ac.size() - 3] == '/'
        )
        {
            ac.pop_back();
            ac.pop_back();
            ac.pop_back();
            ac.pop_back();
            assert(!ac.empty());
            while (!ac.empty() && ac.back() != '/')
                ac.pop_back();
            continue;
        }

        //
        // see if the path so far is a symbolic link
        //
#ifdef S_IFLNK
        if (resolve)
        {
            string_ty *s = ac.mkstr();
            char pointer[2000];
            int nbytes =
                glue_readlink(s->str_text, pointer, sizeof(pointer) - 1);
            if (nbytes < 0)
            {
                int errno_old = errno;

                //
                // probably not a symbolic link
                //
                if
                (
                    //
                    // Some broken Unixes say this when they should
                    // say EINVAL.
                    //
                    errno_old != ENXIO
                &&
                    //
                    // The file definitely isn't a symbolic link.
                    //
                    errno_old != EINVAL
                &&
                    //
                    // The file doesn't exist.
                    //
                    errno_old != ENOENT
                &&
                    //
                    // One of the path elements isn't a directory.
                    //
                    errno_old != ENOTDIR
                &&
                    //
                    // If we can't read the link (or more likely, one
                    // of the directories above it wont let us search)
                    // assume it isn't a link.
                    //
                    errno_old != EACCES
                )
                {
                    sub_context_ty  *scp;

                    scp = sub_context_new();
                    sub_errno_setx(scp, errno_old);
                    sub_var_set_string(scp, "File_Name", s);
                    fatal_intl(scp, i18n("readlink $filename: $errno"));
                    // NOTREACHED
                }
                str_free(s);
            }
            else
            {
                string_ty       *newpath;
                string_ty       *link1;

                if (nbytes == 0)
                {
                    pointer[0] = '.';
                    nbytes = 1;
                }
                if (++loop > 1000)
                {
                    sub_context_ty  *scp;

                    scp = sub_context_new();
                    sub_errno_setx(scp, ELOOP);
                    sub_var_set_string(scp, "File_Name", s);
                    fatal_intl(scp, i18n("readlink $filename: $errno"));
                    // NOTREACHED
                }
                link1 = str_n_from_c(pointer, nbytes);
#ifdef HAVE_CLU_INFO
                {
                    string_ty       *link2;

                    //
                    // OSF/1 has magic symlinks,
                    // where the string "{memb}"
                    // is replaced by the cluster
                    // member name.
                    //
                    // This is like the DG/UX "elink"
                    // concept, where environment
                    // variables can be substituted
                    // into symlinks.
                    //
                    link2 = magic_memb_replace(link1);
                    str_free(link1);
                    link1 = link2;
                }
#endif
                str_free(s);
                if (link1->str_text[0] == '/')
                {
                    newpath =
                        str_format
                        (
                            "%s/%s",
                            link1->str_text,
                            path->str_text + ipos
                        );
                }
                else
                {
                    while (ac.back() != '/')
                        ac.pop_back();

                    newpath =
                        str_format
                        (
                            "%.*s/%s/%s",
                            int(ac.size()),
                            ac.get_data(),
                            link1->str_text,
                            path->str_text + ipos
                        );
                }
                str_free(link1);
                str_free(path);
                path = newpath;
                ipos = 0;
                ac.clear();
                found = 0;
                continue;
            }
        }
#endif

        //
        // keep the slash
        //
        ac.push_back(c);
    }
    str_free(path);
    assert(!ac.empty());
    assert(ac[0] == '/');
    assert(ac.back() == '/');
    if (ac.size() >= 2)
        ac.pop_back();
    path = ac.mkstr();
    trace_string(path->str_text);

    //
    // Check for automounter prefixes, and remove them if you
    // find them.  The user needs to use this sparingly, because
    // extreme chaos can result.
    //
    result = remove_automounter_prefix(path);
    str_free(path);
    trace_string(result->str_text);
    trace(("}\n"));
    return result;
}


nstring
os_pathname(const nstring &path, bool resolve)
{
    return nstring(os_pathname(path.get_ref(), (int)resolve));
}


// vim: set ts=8 sw=4 et :
