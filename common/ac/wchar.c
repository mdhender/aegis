/*
 *	aegis - project change supervisor
 *	Copyright (C) 1996, 1999 Peter Miller;
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
 * MANIFEST: impliment missing functions from <wchar.h>
 */

#include <ac/stddef.h>
#include <ac/wchar.h>


#ifndef HAVE_WCSLEN


size_t
wcslen(wcs)
	const wchar_t	*wcs;
{
	const wchar_t	*start;

	start = wcs;
	while (*wcs)
		++wcs;
	return (wcs - start);
}


#endif /* !HAVE_WCSLEN */


/*
 * NAME
 *	column_width - determine column width of a wide character
 *
 * SYNOPSIS
 *	int column_width(wint_t);
 *
 * DESCRIPTION
 * 	The column_width function is used to determine the column width
 * 	if a wide character.  This is particularly hard to do,
 * 	especially if you have read the ISO C standard ammendments.
 *
 * WEASEL WORDS
 *	This is the phrase used by P.J. Plauger in his CUJ columns about
 *	standard C and C++, specially when the standard dances all
 *	around the issue, rather than actually solving anything.  Take a
 *	squiz at these classic weasel words...
 *
 *	In the original standard, ISO/IEC 9899:1990, 7.3 Character
 *	handling <ctype.h> reads as follows (3rd paragraph):
 *
 *		The term <i>printing character</i> refers to a member of
 *		an implemntation defined set of characters, each of
 *		which occupies one printing position on a display
 *		device; the term <i>control character</i> refers to a
 *		member of an implementation defined set of characters
 *		that are not printing characters.
 *
 *	The following 2 sections are from ISO/IEC 9899:1990/Amd. 1:1995 (E):
 *
 *	7.15.2 Wide-character classification utilities (2nd paragraph)
 *
 *		The term <i>printing wide character</i> refers to a
 *		member of a locale-specific set of wide characters, each
 *		of which occupies at least one printing position on a
 *		display device; the term <i>control wide character</i>
 *		refers to a member of a locale-specific set of wide
 *		characters that are not printing wide characters.
 *
 *	[ Notice how they weasel out by not-quite contradicting 7.3: a
 *	printing <i>char</i> is exactly one printing position wide, but
 *	a printing <i>wchar_t</i> is one or more printing positions
 *	wide. ]
 *
 *	H.14 Column width
 *
 *		The number of characters to be read or written can be
 *		specified in existing formatted i/o functions.  On a
 *		traditional display device that displays characters with
 *		fixed pitch, the number of characters is directly
 *		proportional to the width occupied by the characters.
 *		So the display format can be specified through the field
 *		width and/or the precision.
 *
 *		In formatted wide-character i/o functions, the field
 *		width and the precision specify the number of wide
 *		characters to be read or written.  The number of wide
 *		characters is not always directly proportional to the
 *		width of their display.  For example, with Japanese
 *		traditional display devices, a single-byte character
 *		such as an ASCII character has half the width of a Kanji
 *		character, even though each of them is treated as one
 *		wide character.  To control the display format for wide
 *		characters, a set of formatted wide-character i/o
 *		functions were proposed whose metric was the column
 *		width instead of the character count.
 *
 *		This proposal was supported only by Japan.  Critics
 *		observed that the proposal was based on such traditional
 *		display devices with a fixed width of characters, while
 *		many modern display devices support a broad assortment
 *		of proportional pitch type faces.  Hence, it was
 *		questioned whether the extra i/o functions in this
 *		proposal were really needed or were sufficiently
 *		general.  Also considered were another set of functions
 *		that return the column width for any kind of display
 *		devices for a given wide-character string; but these
 *		seemed to be beyond the scope of the C language.  Thus
 *		all proposals regarding column width were withdrawn.
 *
 *	[ Notice how 7.15.2 specifically states that each printing
 *	character has a non-zero width measurable in <i>printing
 *	positions</i>.  Why is this metric is unavailable to the
 *	C programmer?  Presumably it is OK for an informational appendix
 *	to contradict the body of the standard. ]
 *
 *	[ The section ends with a compliant-but-non-standard way a
 *	standard C library implementor may choose to do this.  You can't
 *	relie on it being there, and you can't relie on the suggested
 *	semantics being used, so don't even bother having ./configure go
 *	look for it. ]
 *
 * SO FAKE IT
 *	Since there is no standard way to determine character width, we
 *	will have to fake it.  Hopefully, locales that need it will
 *	define something useful.  If you know of any, please let me
 *	know.
 *
 *	The wcwidth and wcswidth functions are extensions found in
 *	X/Open CAE.  So, we use the same function names as they did,
 *	when we need to fake it (which is all of the time, if you don't
 *	have glibc).
 */

#ifndef HAVE_WCWIDTH

#ifdef HAVE_ISWCTYPE
#ifdef HAVE_WCTYPE_H
#include <ac/wctype.h>
#endif
#endif


int
wcwidth(wc)
	wint_t		wc;
{
#ifdef HAVE_ISWCTYPE
#ifdef HAVE_WCTYPE_H
	static int	kanji_set;
	static wctype_t	kanji;

	if (!kanji_set)
	{
		kanji = wctype("kanji");
		kanji_set = 1;
	}
	if (kanji && iswctype(kanji, wc))
		return 2;
#endif
#endif
	return 1;
}


int
wcswidth(wcs, n)
	wchar_t		*wcs;
	size_t		n;
{
	int		result;

	result = 0;
	while (*wcs && n > 0)
	{
		result += wcwidth(*wcs++);
		--n;
	}
	return result;
}

#endif
