//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/abbreviate.h>
#include <libaegis/change.h>
#include <common/error.h>	// for assert
#include <common/gmatch.h>
#include <libaegis/project/file.h>
#include <common/str.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>


static int
change_maximum_filename_length(change::pointer cp)
{
    pconf_ty        *pconf_data;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->maximum_filename_length);
    return pconf_data->maximum_filename_length;
}


static bool
contains_moronic_ms_restrictions(string_ty *fn)
{
    static const char *const moronic[] =
    {
	"[aA][uU][xX]",
	"[aA][uU][xX].*",
	"[cC][oO][mM][1-4]",
	"[cC][oO][mM][1-4].*",
	"[cC][oO][nN]",
	"[cC][oO][nN].*",
	"[nN][uU][lL]",
	"[nN][uU][lL].*",
    };
    const char      *const *cpp;

    for (cpp = moronic; cpp < ENDOF(moronic); ++cpp)
	if (gmatch(*cpp, fn->str_text))
	    return true;
    return false;
}


static bool
is_a_dos_filename(change::pointer cp, string_ty *fn)
{
    pconf_ty        *pconf_data;
    string_list_ty  wl;
    size_t          j;

    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->dos_filename_required)
	return true;

    //
    // watch out for the moronic ms restrictions
    //
    if (contains_moronic_ms_restrictions(fn))
	return false;

    //
    // make sure the filename
    //      1. contains at most one dot
    //      2. contains a non-empty prefix of at most 8 characters
    //      3. optionally contains a non-empty suffix of at
    //         most 3 characters (reject empty suffix, because DOS
    //         will think that "foo" and "foo." are the same name)
    //
    wl.split(fn, ".");
    if (wl.nstrings < 1 || wl.nstrings > 2)
	return false;
    if (wl.string[0]->str_length < 1 || wl.string[0]->str_length > 8)
	return false;
    if
    (
	wl.nstrings > 1
    &&
	(wl.string[1]->str_length < 1 || wl.string[1]->str_length > 3)
    )
	return false;

    //
    // make sure the characters are acceptable
    // only allow alphanumerics
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
	for (const char *tp = wl.string[j]->str_text; *tp; ++tp)
	{
	    if (!isalnum((unsigned char)*tp))
		return false;
	}
    }
    return true;
}


static bool
is_a_windows_filename(change::pointer cp, string_ty *fn)
{
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->windows_filename_required)
	return true;

    //
    // watch out for the moronic ms restrictions
    //
    if (contains_moronic_ms_restrictions(fn))
	return false;

    //
    // make sure the characters are acceptable
    //
    return (0 == strpbrk(fn->str_text, ":\"'\\"));
}


static int
filename_pattern_test(change::pointer cp, string_ty *fn)
{
    pconf_ty        *pconf_data;
    size_t          j;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);

    //
    // check rejections first
    //
    assert(pconf_data->filename_pattern_reject);
    for (j = 0; j < pconf_data->filename_pattern_reject->length; ++j)
    {
	string_ty       *s;

	s = pconf_data->filename_pattern_reject->list[j];
	if (gmatch(s->str_text, fn->str_text))
	    return 0;
    }

    //
    // check acceptable patterns
    //  (defaults to "*" if not set, or set but empty)
    //
    assert(pconf_data->filename_pattern_accept);
    assert(pconf_data->filename_pattern_accept->length);
    for (j = 0; j < pconf_data->filename_pattern_accept->length; ++j)
    {
	string_ty       *s;

	s = pconf_data->filename_pattern_accept->list[j];
	if (gmatch(s->str_text, fn->str_text))
	    return 1;
    }

    //
    // did not match any accept pattern
    //
    return 0;
}


static int
change_filename_in_charset(change::pointer cp, string_ty *fn)
{
    pconf_ty        *pconf_data;
    char            *s;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    if (pconf_data->posix_filename_charset)
    {
	//
	// For a filename to be portable across conforming
	// implementations of IEEE Std 1003.1-1988, it shall
	// consist only of the following characters.  Hyphen
	// shall not be used as the first character of a
	// portable filename.
	//
	if (fn->str_text[0] == '-')
	    return 0;
	for (s = fn->str_text; *s; ++s)
	{
	    switch (*s)
	    {
	    default:
		return 0;

	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
	    case 'f':
	    case 'g':
	    case 'h':
	    case 'i':
	    case 'j':
	    case 'k':
	    case 'l':
	    case 'm':
	    case 'n':
	    case 'o':
	    case 'p':
	    case 'q':
	    case 'r':
	    case 's':
	    case 't':
	    case 'u':
	    case 'v':
	    case 'w':
	    case 'x':
	    case 'y':
	    case 'z':
	    case 'A':
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'F':
	    case 'G':
	    case 'H':
	    case 'I':
	    case 'J':
	    case 'K':
	    case 'L':
	    case 'M':
	    case 'N':
	    case 'O':
	    case 'P':
	    case 'Q':
	    case 'R':
	    case 'S':
	    case 'T':
	    case 'U':
	    case 'V':
	    case 'W':
	    case 'X':
	    case 'Y':
	    case 'Z':
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	    case '-':
	    case '.':
	    case '_':
		break;
	    }
	}
    }
    else
    {
	//
	// exclude control characters, space characters
	// and high-bit-on characters
	// (This is a "C" locale test.)
	//
	for (s = fn->str_text; *s; ++s)
	    if (!isgraph((unsigned char)*s))
		return 0;
    }
    return 1;
}


static bool
change_filename_shell_safe(change::pointer cp, string_ty *fn)
{
    pconf_ty        *pconf_data;
    char            *s;

    //
    // Though not a shell special character as such, always disallow
    // a leading minus.  This is because it is the option introducer,
    // and will cause confusion if allowed into filenames.  Also,
    // there is no elegant and context-free way to quote it.
    //
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    if (fn->str_text[0] == '-')
	return false;
    if (!pconf_data->shell_safe_filenames)
	return true;

    //
    // Some shells also treat a leading tilde (~) as meaning "home
    // directory of"
    //
    if (fn->str_text[0] == '~')
	return false;

    //
    // The rest of the restrictions apply to all characters of
    // the filename.
    //
    for (s = fn->str_text;; ++s)
    {
	switch ((unsigned char)*s)
	{
	default:
	    continue;

	case 0:
	    break;

        case '\t':
        case '\n':
        case ' ':
	case '!':
	case '"':
	case '#':
	case '$':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '?':
	case '[':
	case '\\':
	case ']':
	case '^':
	case '`':
	case '{':
	case '|':
	case '}':
	    return false;
	}
	break;
    }
    return true;
}


static bool
contains_white_space(change::pointer cp, string_ty *fn)
{
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (pconf_data->allow_white_space_in_filenames)
	return false;
    return (0 != strpbrk(fn->str_text, "\b\t\n\v\f\r "));
}


static bool
contains_non_ascii(change::pointer cp, string_ty *fn)
{
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (pconf_data->allow_non_ascii_filenames)
	return false;
    const char *s = fn->str_text;
    const char *end = s + fn->str_length;
    while (s < end)
    {
        unsigned char c = *s++;
        switch (c)
        {
        default:
            return true;

        case '\b':
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
        case '!':
        case '"':
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
        case '-':
        case '.':
        case '/':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case ':':
        case ';':
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '_':
        case '`':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '{':
        case '|':
        case '}':
        case '~':
            break;
        }
    }
    return false;
}


//
// NAME
//	change_filename_check
//
// SYNOPSIS
//	string_ty *change_filename_check(change::pointer cp, string_ty *fn);
//
// DESCRIPTION
//      The change_filename_check function is used to determine if a
//      filename is acceptable, accirding to the various filename
//	constraints.
//	1. it must not have any element which is too long
//	2. it must not have any element which ends in ,D
//	3. no directory may duplicate an existing file
//	4. you can't call it the name of the default log file
//	5. it must not have any illegal characters
//
// ARGUMENTS
//	cp	- the change the filename is related to
//	fn	- the filename to check
//
// RETURNS
//	0 on success, otherwise a pointer to an error message.
//	The error message will need to be freed after printing.
//

string_ty *
change_filename_check(change::pointer cp, string_ty *filename)
{
    string_list_ty  part;
    size_t          k;
    string_ty       *result;
    string_ty       *s2;

    //
    // figure the limits
    // name_max_dir is for directories
    // name_max_fil is for the basename
    //
    // The margin of 2 on the end is for ",D" suffixes,
    // and for ",v" in RCS, "s." in SCCS, etc.
    //
    trace(("change_filename_check(cp = %08lX, filename = \"%s\")\n{\n",
	(long)cp, filename->str_text));
    assert(cp->reference_count >= 1);
    int name_max_dir = change_maximum_filename_length(cp);
    int name_max_fil = change_pathconf_name_max(cp);
    if (name_max_dir > name_max_fil)
	name_max_dir = name_max_fil;
    name_max_fil -= 2;
    if (name_max_fil > name_max_dir)
	name_max_fil = name_max_dir;

    //
    // break into path elements
    // and check each element
    //
    part.split(filename, "/");
    result = 0;
    for (k = 0; k < part.nstrings; ++k)
    {
	s2 = part.string[k];

        //
        // Check for white space characters
        //
        if (contains_white_space(cp, s2))
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", filename);
            result =
                sc.subst_intl
                (
                    i18n("file name \"$filename\" contains illegal characters")
                );
            goto done;
        }

        //
        // check for non-white-space not-ascii-printable characters
        // (i.e. probably UTF8 or something)
        //
        if (contains_non_ascii(cp, s2))
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", filename);
            result =
                sc.subst_intl
                (
                    i18n("file name \"$filename\" contains illegal characters")
                );
            goto done;
        }

	//
	// check DOS-full-ness
	//
	if (!is_a_dos_filename(cp, s2))
	{
	    sub_context_ty  *scp;
	    string_ty       *s3;

	    s3 = abbreviate_8dos3(s2);
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    sub_var_set_string(scp, "SUGgest", s3);
	    sub_var_optional(scp, "SUGgest");
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl
		    (
			scp,
			i18n("file name \"$filename\" not suitable for DOS")
		    );
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
		  i18n("file \"$filename\" part \"$part\" not suitable for DOS")
		    );
	    }
	    sub_context_delete(scp);
	    str_free(s3);
	    goto done;
	}
	if (!is_a_windows_filename(cp, s2))
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl
		    (
			scp,
			i18n("file name \"$filename\" not suitable for Windows")
		    );
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
	      i18n("file \"$filename\" part \"$part\" not suitable for Windows")
		    );
	    }
	    sub_context_delete(scp);
	    goto done;
	}

	//
	// check name length
	//
        int name_max = name_max_dir;
	if (k == part.nstrings - 1)
	    name_max = name_max_fil;
	if (s2->str_length > (size_t)name_max)
	{
	    sub_context_ty  *scp;
	    string_ty       *s3;

	    scp = sub_context_new();
	    if (k == part.nstrings - 1)
		s3 = abbreviate_filename(s2, name_max);
	    else
		s3 = abbreviate_dirname(s2, name_max);
	    sub_var_set_string(scp, "File_Name", filename);
	    sub_var_set_long(scp, "Number", (int)(s2->str_length - name_max));
	    sub_var_optional(scp, "Number");
	    sub_var_set_string(scp, "SUGgest", s3);
	    sub_var_optional(scp, "SUGgest");
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl
		    (
			scp,
	       i18n("file \"$filename\" too long, suggest \"$suggest\" instead")
		    );
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
i18n("file \"$filename\" part \"$part\" too long, suggest \"$suggest\" instead")
		    );
	    }
	    sub_context_delete(scp);
	    str_free(s3);
	    goto done;
	}

	//
	// check for logfile
	//
	if (str_equal(s2, change_logfile_basename()))
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl(scp, i18n("file \"$filename\" is log file"));
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
			i18n("file \"$filename\" part \"$part\" is log file")
		    );
	    }
	    sub_context_delete(scp);
	    goto done;
	}

	//
	// check filename for valid characters
	//
	if (!change_filename_in_charset(cp, s2))
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl
		    (
			scp,
		     i18n("file name \"$filename\" contains illegal characters")
		    );
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
	   i18n("file \"$filename\" part \"$part\" contains illegal characters")
		    );
	    }
	    sub_context_delete(scp);
	    goto done;
	}

	//
	// check filename for shell special characters
	//
	if (!change_filename_shell_safe(cp, s2))
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl
		    (
			scp,
	       i18n("file name \"$filename\" contains shell special characters")
		    );
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
     i18n("file \"$filename\" part \"$part\" contains shell special characters")
		    );
	    }
	    sub_context_delete(scp);
	    goto done;
	}
	if (!filename_pattern_test(cp, s2))
	{
	    sub_context_ty *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    if (part.nstrings == 1)
	    {
		result =
		    subst_intl(scp, i18n("file \"$filename\" bombs filters"));
	    }
	    else
	    {
		sub_var_set_string(scp, "Part", s2);
		result =
		    subst_intl
		    (
			scp,
		        i18n("file \"$filename\" part \"$part\" bombs filters")
		    );
	    }
	    sub_context_delete(scp);
	    goto done;
	}
    }

    //
    // here for all exits
    //
    done:
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
