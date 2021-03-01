/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate name_checks
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <abbreviate.h>
#include <change.h>
#include <error.h> /* for assert */
#include <gmatch.h>
#include <project/file.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>


static int change_maximum_filename_length _((change_ty *));

static int
change_maximum_filename_length(cp)
	change_ty	*cp;
{
	pconf		pconf_data;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->maximum_filename_length);
	return pconf_data->maximum_filename_length;
}


static int contains_moronic_ms_restrictions _((string_ty *));

static int
contains_moronic_ms_restrictions(fn)
	string_ty	*fn;
{
	static char	*moronic[] =
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
	char		**cpp;

	for (cpp = moronic; cpp < ENDOF(moronic); ++cpp)
		if (gmatch(*cpp, fn->str_text))
			return 1;
	return 0;
}


static int is_a_dos_filename _((change_ty *, string_ty *));

static int
is_a_dos_filename(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	string_list_ty	wl;
	int		result;
	size_t		j;

	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->dos_filename_required)
		return 1;

	/*
	 * watch out for the moronic ms restrictions
	 */
	if (contains_moronic_ms_restrictions(fn))
		return 0;

	/*
	 * make sure the filename
	 *	1. contains at most one dot
	 *	2. contains a non-empty prefix of at most 8 characters
	 *	3. optionally contains a non-empty suffix of at
	 *	   most 3 characters (reject empty suffix, because DOS
	 *	   will think that "foo" and "foo." are the same name)
	 */
	str2wl(&wl, fn, ".", 0);
	result = 0;
	if (wl.nstrings < 1 || wl.nstrings > 2)
		goto done;
	if (wl.string[0]->str_length < 1 || wl.string[0]->str_length > 8)
		goto done;
	if
	(
		wl.nstrings > 1
	&&
		(wl.string[1]->str_length < 1 || wl.string[1]->str_length > 3)
	)
		goto done;

	/*
	 * make sure the characters are acceptable
	 * only allow alphanumerics
	 */
	for (j = 0; j < wl.nstrings; ++j)
	{
		char	*tp;

		for (tp = wl.string[j]->str_text; *tp; ++tp)
		{
			if (!isalnum((unsigned char)*tp))
				goto done;
		}
	}
	result = 1;

	done:
	string_list_destructor(&wl);
	return result;
}


static int is_a_windows_filename _((change_ty *, string_ty *));

static int
is_a_windows_filename(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	char		*s;

	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->windows_filename_required)
		return 1;

	/*
	 * watch out for the moronic ms restrictions
	 */
	if (contains_moronic_ms_restrictions(fn))
		return 0;

	/*
	 * make sure the characters are acceptable
	 */
	for (s = fn->str_text; *s; ++s)
	{
		if (strchr(":\"'\\", *s))
			return 0;
	}

	/*
	 * didn't find anything wrong
	 */
	return 1;
}


static int filename_pattern_test _((change_ty *, string_ty *));

static int
filename_pattern_test(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	size_t		j;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);

	/*
	 * check rejections first
	 */
	assert(pconf_data->filename_pattern_reject);
	for (j = 0; j < pconf_data->filename_pattern_reject->length; ++j)
	{
		string_ty	*s;

		s = pconf_data->filename_pattern_reject->list[j];
		if (gmatch(s->str_text, fn->str_text))
			return 0;
	}

	/*
	 * check acceptable patterns
	 *  (defaults to "*" if not set, or set but empty)
	 */
	assert(pconf_data->filename_pattern_reject);
	assert(pconf_data->filename_pattern_reject->length);
	for (j = 0; j < pconf_data->filename_pattern_reject->length; ++j)
	{
		string_ty	*s;

		s = pconf_data->filename_pattern_accept->list[j];
		if (gmatch(s->str_text, fn->str_text))
			return 1;
	}

	/*
	 * did not match any accept pattern
	 */
	return 0;
}


static int change_filename_in_charset _((change_ty *, string_ty *));

static int
change_filename_in_charset(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	char		*s;

	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (pconf_data->posix_filename_charset)
	{
		/*
		 * For a filename to be portable across conforming
		 * implementations of IEEE Std 1003.1-1988, it shall
		 * consist only of the following characters.  Hyphen
		 * shall not be used as the first character of a
		 * portable filename.
		 */
		if (fn->str_text[0] == '-')
			return 0;
		for (s = fn->str_text; *s; ++s)
		{
			switch (*s)
			{
			default:
				return 0;

			case 'a': case 'b': case 'c': case 'd': case 'e':
			case 'f': case 'g': case 'h': case 'i': case 'j':
			case 'k': case 'l': case 'm': case 'n': case 'o':
			case 'p': case 'q': case 'r': case 's': case 't':
			case 'u': case 'v': case 'w': case 'x': case 'y':
			case 'z':
			case 'A': case 'B': case 'C': case 'D': case 'E':
			case 'F': case 'G': case 'H': case 'I': case 'J':
			case 'K': case 'L': case 'M': case 'N': case 'O':
			case 'P': case 'Q': case 'R': case 'S': case 'T':
			case 'U': case 'V': case 'W': case 'X': case 'Y':
			case 'Z':
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			case '-': case '.': case '_':
				break;
			}
		}
	}
	else
	{
		/*
		 * exclude control characters, space characters
		 * and high-bit-on characters
		 * (This is a "C" locale test.)
		 */
		for (s = fn->str_text; *s; ++s)
			if (!isgraph((unsigned char)*s))
				return 0;
	}
	return 1;
}


static int change_filename_shell_safe _((change_ty *, string_ty *));

static int
change_filename_shell_safe(cp, fn)
	change_ty	*cp;
	string_ty	*fn;
{
	pconf		pconf_data;
	char		*s;

	/*
	 * Though not a shell special character as such, always disallow
	 * a leading minus.  This is because it is the option introducer,
	 * and will cause confusion if allowed into filenames.	Also,
	 * there is no elegant and context-free way to quote it.
	 */
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (fn->str_text[0] == '-')
		return 0;
	if (!pconf_data->shell_safe_filenames)
		return 1;

	/*
	 * Some shells also treat a leading tilde (~) as meaning ``home
	 * directory of''
	 */
	if (fn->str_text[0] == '~')
		return 0;

	/*
	 * The rest of the restrictions apply to all characters of
	 * the filename.
	 */
	for (s = fn->str_text; ; ++s)
	{
		switch ((unsigned char)*s)
		{
		default:
			continue;

		case 0:
			break;

		case '!': case '"': case '#': case '$': case '&': case '\'':
		case '(': case ')': case '*': case ':': case ';': case '<':
		case '=': case '>': case '?': case '[': case '\\': case ']':
		case '^': case '`': case '{': case '|': case '}':
			return 0;
		}
		break;
	}
	return 1;
}


/*
 * NAME
 *	change_filename_check
 *
 * SYNOPSIS
 *	string_ty *change_filename_check(change_ty *cp, string_ty *fn);
 *
 * DESCRIPTION
 *      The change_filename_check function is used to determine if a
 *      filename is acceptable, accirding to the various filename
 *	constraints.
 *	1. it must not have any element which is too long
 *	2. it must not have any element which ends in ,D
 *	3. no directory may duplicate an existing file
 *	4. you can't call it the name of the default log file
 *	5. it must not have any illegal characters
 *
 * ARGUMENTS
 *	cp	- the change the filename is related to
 *	fn	- the filename to check
 *
 * RETURNS
 *	0 on success, otherwise a pointer to an error message.
 *	The error message will need to be freed after printing.
 */

string_ty *
change_filename_check(cp, filename, nodup)
	change_ty	*cp;
	string_ty	*filename;
	int		nodup;
{
	int		name_max1;
	int		name_max2;
	string_list_ty	part;
	int		k;
	string_ty	*result;
	string_ty	*s2;

	/*
	 * figure the limits
	 * name_max1 is for directories
	 * name_max2 is for the basename
	 *
	 * The margin of 2 on the end is for ",D" suffixes,
	 * and for ",v" in RCS, "s." in SCCS, etc.
	 */
	trace(("change_filename_check(cp = %8l8lX, filename = \"%s\", nodup = %d)\n{\n"/*}*/,
		(long)cp, filename->str_text, nodup));
	assert(cp->reference_count >= 1);
	name_max1 = change_maximum_filename_length(cp);
	name_max2 = change_pathconf_name_max(cp);
	if (name_max1 > name_max2)
		name_max1 = name_max2;
	name_max2 -= 2;
	if (name_max2 > name_max1)
		name_max2 = name_max1;

	/*
	 * break into path elements
	 * and check each element
	 */
	str2wl(&part, filename, "/", 0);
	result = 0;
	for (k = 0; k < part.nstrings; ++k)
	{
		int		max;

		/*
		 * check for collision
		 */
		if (nodup)
		{
			fstate_src	src_data;

			s2 = wl2str(&part, 0, k, "/");
			src_data = project_file_find(cp->pp, s2);
			if
			(
				src_data
			&&
				!src_data->deleted_by
			&&
				!src_data->about_to_be_created_by
			)
			{
				if
				(
					part.nstrings == 1
				||
					str_equal(s2, filename)
				)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name", "%S", filename);
					result =
						subst_intl
						(
							scp,
			    i18n("file \"$filename\" already exists in project")
						);
					sub_context_delete(scp);
				}
				else
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_var_set(scp, "File_Name1", "%S", filename);
					sub_var_set(scp, "File_Name2", "%S", s2);
					result =
						subst_intl
						(
							scp,
	i18n("file \"$filename1\" collides with file \"$filename2\" in project")
						);
					sub_context_delete(scp);
				}
				str_free(s2);
				goto done;
			}
			str_free(s2);
		}

		/*
		 * check DOS-full-ness
		 */
		s2 = part.string[k];
		if (!is_a_dos_filename(cp, s2))
		{
			sub_context_ty	*scp;
			string_ty	*s3;

			s3 = abbreviate_8dos3(s2);
			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			sub_var_set(scp, "SUGgest", "%S", s3);
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
				sub_var_set(scp, "Part", "%S", s2);
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
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
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
				sub_var_set(scp, "Part", "%S", s2);
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

		/*
		 * check name length
		 */
		if (k == part.nstrings - 1)
			max = name_max2;
		else
			max = name_max1;
		if (s2->str_length > max)
		{
			sub_context_ty	*scp;
			string_ty	*s3;

			scp = sub_context_new();
			if (k == part.nstrings - 1)
				s3 = abbreviate_filename(s2, max);
			else
				s3 = abbreviate_dirname(s2, max);
			sub_var_set(scp, "File_Name", "%S", filename);
			sub_var_set(scp, "Number", "%d", (int)(s2->str_length - max));
			sub_var_optional(scp, "Number");
			sub_var_set(scp, "SUGgest", "%S", s3);
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
				sub_var_set(scp, "Part", "%S", s2);
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

		/*
		 * check for logfile
		 */
		if (str_equal(s2, change_logfile_basename()))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
					  i18n("file \"$filename\" is log file")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
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

		/*
		 * check filename for valid characters
		 */
		if (!change_filename_in_charset(cp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
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
				sub_var_set(scp, "Part", "%S", s2);
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

		/*
		 * check filename for shellspecial characters
		 */
		if (!change_filename_shell_safe(cp, s2))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
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
				sub_var_set(scp, "Part", "%S", s2);
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
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", filename);
			if (part.nstrings == 1)
			{
				result =
					subst_intl
					(
						scp,
					i18n("file \"$filename\" bombs filters")
					);
			}
			else
			{
				sub_var_set(scp, "Part", "%S", s2);
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

	/*
	 * here for all exits
	 */
	done:
	string_list_destructor(&part);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}
