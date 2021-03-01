//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/error.h>
#include <libaegis/os.h>
#include <common/trace.h>


//
// NAME
//	change_fingerprint_same
//
// SYNOPSIS
//	int change_fingerprint_same(fingerprint fp, string_ty *path);
//
// DESCRIPTION
//	The change_fingerprint_same function is used to test if a file
//	fingerprint is the same.  This implies the file itself is the
//	same.  If the file does not exist, it is aas if the fingerprint
//	hash changed.
//
// RETURNS
//	int;	1 -> the file is the SAME
//		0 -> the file has changed
//

int
change_fingerprint_same(fingerprint_ty *fp, string_ty *path, int check_always)
{
	time_t		oldest;
	time_t		newest;
	string_ty	*crypto;

	//
	// a NULL pointer means something very weird is going on
	//
	trace(("change_fingerprint_same(fp = %8.8lX, path = \"%s\")\n{\n",
	    (long)fp, path->str_text));
	if (!fp)
	{
		trace(("No existing fingerprint\n"));
		trace(("return 0;\n"));
		trace(("}\n"));
		return 0;
	}
	assert(fp->youngest >= 0);
	assert(fp->oldest >= 0);

	//
	// If the file does not exist, clear the fingerprint and say
	// that the file "is not the same".
	//
	if (!os_exists(path))
	{
		fp->youngest = 0;
		fp->oldest = 0;
		if (fp->crypto)
		{
			str_free(fp->crypto);
			fp->crypto = 0;
		}
		trace(("no file there to fingerprint\n"));
		trace(("return 0;\n"));
		trace(("}\n"));
		return 0;
	}

	//
	// The youngest field and the file's mtime should be the same;
	// if it is, don't bother checking the fingerprint, just say the
	// the file is the same.  (The os_mtime function checks the
	// ctime, too, just in case the user is trying to fake us out.)
	//
	os_mtime_range(path, &oldest, &newest);
	assert(oldest > 0);
	assert(newest > 0);
	if
	(
		!check_always
	&&
		fp->crypto
	&&
		fp->oldest
	&&
		fp->youngest
	&&
		fp->youngest == newest
	)
	{
		trace(("file times match\n"));
		if (oldest < fp->oldest)
			fp->oldest = oldest;
		trace(("return 1;\n"));
		trace(("}\n"));
		return 1;
	}

	//
	// Read the fingerprint.  If it is the same as before,
	// extend the valid time range, and say the file is the same.
	//
	crypto = os_fingerprint(path);
	if
	(
		fp->crypto
	&&
		fp->oldest
	&&
		fp->youngest
	&&
		str_equal(crypto, fp->crypto)
	)
	{
		trace(("file fingerprints match\n"));
		str_free(crypto);
		if (newest > fp->youngest)
			fp->youngest = newest;
		if (oldest < fp->oldest)
			fp->oldest = oldest;
		trace(("return 1;\n"));
		trace(("}\n"));
		return 1;
	}

	//
	// Everything has changed, reset everything and then say the
	// file is not the same.
	//
	fp->oldest = oldest;
	fp->youngest = newest;
	if (fp->crypto)
		str_free(fp->crypto);
	fp->crypto = crypto;
	trace(("file fingerprint mis-match\n"));
	trace(("return 0;\n"));
	trace(("}\n"));
	return 0;
}
