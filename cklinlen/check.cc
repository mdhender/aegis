//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <cklinlen/check.h>
#include <common/error.h>

bool            warning;
static int      limit;
static int      number_of_blank_lines;
static int      number_of_errors;
static int      line_number;
static bool     dos_format;
static bool     binary_format;
static FILE     *fp;
static const char *fn;
static bool     isa_c_file;
static bool     isa_cxx_file;
static bool     isa_h_file;
static int      unprintable_ok;


enum state_t
{
    state_normal,
    state_double_quote,
    state_double_quote_escape,
    state_single_quote,
    state_single_quote_escape,
    state_slash,
    state_cxx_comment,
    state_c_comment,
    state_c_comment_begin,
    state_c_comment_doxygen,
    state_c_comment_star
};

static state_t state;


static void
check_c_comment(void)
{
    if (isa_h_file && !isa_c_file)
    {
	isa_h_file = false;
	isa_c_file = true;
    }
    if (isa_cxx_file)
    {
	fprintf(stderr, "%s: %d: C comment in a C++ file\n",
	    fn, line_number);
	++number_of_errors;
    }
}


//
// The run_state_machine function is used to track the C and C++
// comment state of the file.  This allows us to generate warnings
// about inappropriate comments.
//
// All C++ compilers accept C comments, and many C compilers accept C++
// comments (and a recent standard extention permits them).
// However, file_check warns about cross-language comments.
//
// The rationale of this is that the comments give the human reader
// an important clue as to which language they should expect to
// be reading.  While the semantic differences between C and C++ are
// obviously different in many cases, in some cases they are merely
// subltly different.  Not all C code works identically in C++.
// Consistent use of the approriate comment form gives the human reader
// an obvious reminder.
//
// Exception: The Doxygen and DOC++ comment form is / * * blah * /,
// and this is accepted in C++, mostly because Doxygen and DOC++ don't
// treat runs of /// as a single block comment.  Usually such comments
// are isolated in the obviously different class declarations, so should
// not be an issue.
//

static void
run_state_machine(int c)
{
    if (!isa_c_file && !isa_cxx_file && !isa_h_file)
	return;
    switch (state)
    {
    case state_normal:
	//
	// This state is for the body of a C or C++ file.  We aren't in
	// a string, or a character onstant, or any kind of comment.
	//
	switch (c)
	{
	case '/':
	    state = state_slash;
	    break;

	case '\'':
	    state = state_single_quote;
	    break;

	case '"':
	    state = state_double_quote;
	    break;

	default:
	    break;
	}
	break;

    case state_slash:
	//
	// In this state we have seen a slash.  It could be the start
	// of a C or C++ commant, or just one of the division operators.
	//
	switch (c)
	{
	case '/':
	    //
	    // We have seen the start of a C++ comment.
	    //
	    state = state_cxx_comment;
	    if (isa_h_file && !isa_cxx_file)
	    {
		isa_h_file = true;
		isa_cxx_file = true;
	    }
	    if (isa_c_file)
	    {
		fprintf(stderr, "%s: %d: C++ comment in a C file\n",
		    fn, line_number);
		++number_of_errors;
	    }
	    break;

	case '*':
	    //
	    // We have seen the start of a C comment, but it could be
	    // a Doxygen introducer, so we can't whine if it's a C++
	    // file just yet.
	    //
	    state = state_c_comment_begin;
	    break;

	default:
	    //
	    // One of the division operators.
	    // No need to change state.
	    //
	    state = state_normal;
	    break;
	}
	break;

    case state_double_quote:
	//
	// In this state we have seen a double quote, and possibly
	// some content.  We are waiting for the closing double quote.
	//
	switch (c)
	{
	case '\\':
	    //
	    // Start of an escape sequence.
	    //
	    state = state_double_quote_escape;
	    break;

	case '"':
	case '\n':
	    //
	    // Normal and abnormal string constant termination.
	    //
	    state = state_normal;
	    break;

	default:
	    //
	    // Still in the string.  No need to change state.
	    //
	    break;
	}
	break;

    case state_double_quote_escape:
	//
	// We throw away the character immediately following the
	// backslash.  Escape sequences can be longer than this, but
	// are uninteresting to the state machine.  The only sequences
	// which can confuse the state machine are escaped backslash,
	// escaped double quote and escaped newline.
	//
	state = state_double_quote;
	break;

    case state_single_quote:
	//
	// In this state we have seen a single quote, and possibly
	// some content.  We are waiting for the closing single quote.
	//
	switch (c)
	{
	case '\\':
	    //
	    // Start of an escape sequence.
	    //
	    state = state_single_quote_escape;
	    break;

	case '\'':
	case '\n':
	    //
	    // Normal and abnormal character constant termination.
	    //
	    state = state_normal;
	    break;

	default:
	    //
	    // Still in the character constant.  No need to change state.
	    //
	    break;
	}
	break;

    case state_single_quote_escape:
	//
	// We throw away the character immediately following the
	// backslash.  Escape sequences can be longer than this, but
	// are uninteresting to the state machine.  The only sequences
	// which can confuse the state machine are escaped backslash,
	// escaped single quote and escaped newline.
	//
	state = state_single_quote;
	break;

    case state_cxx_comment:
	//
	// We ahve seen '/', '/', and possubly some content.
	// We are waiting for the newline which finishes the comment.
	//
	if (c == '\n')
	    state = state_normal;
	break;

    case state_c_comment_begin:
	//
	// We have seen '/'and '*'.  We are waiting for '*' which could
	// start a Doxygen comment, or anything else which indicates
	// the start of a normal C comment.
	//
	if (c == '*')
	    state = state_c_comment_doxygen;
	else
	{
	    check_c_comment();
	    state = state_c_comment;
	}
	break;

    case state_c_comment_doxygen:
	//
	// We have seen '/', '*' and '*'.
	//
	switch (c)
	{
	case '/':
	    //
	    // This is the end of a very short normal C comment.
	    //
	    check_c_comment();
	    state = state_normal;
	    break;

	case '*':
	    //
	    // This is the start of a very ugly normal C comment.
	    //
	    check_c_comment();
	    state = state_c_comment_star;
	    break;

	default:
	    //
	    // This is a Doxygen comment.  It is allowed in both C and
	    // C++ files, due to the limitations of Doxygen.  Sigh.
	    //
	    state = state_c_comment_star;
	    break;
	}
	break;

    case state_c_comment:
	//
	// We are in the body of a C comment.  We are waiting for a '*'
	// which could start the comment terminator.
	//
	if (c == '*')
	    state = state_c_comment_star;
	break;

    case state_c_comment_star:
	//
	// We have seen a '*' which could preceed a '/' to finish a C comment.
	//
	switch (c)
	{
	case '/':
	    //
	    // C comment terminator.
	    //
	    state = state_normal;
	    break;

	case '*':
	    //
	    // Almost.  The next '/' will end the comment.
	    //
	    break;

	default:
	    //
	    // No, back to the body of the comment.
	    //
	    state = state_c_comment;
	    break;
	}
	break;
    }
}


static int
check_one_line(void)
{
    int             unprintable;
    int             white_space;
    int             pos;
    int             line_contains_white_space;

    ++line_number;
    pos = 0;
    unprintable = 0;
    white_space = 0;
    line_contains_white_space = 0;
    for (;;)
    {
	int     c;

	c = getc(fp);
	if (c == EOF)
	{
	    if (ferror(fp))
		nfatal("read %s", fn);
	    if (pos)
	    {
		fprintf
                (
                    stderr,
                    "%s: %d: last line has no newline\n",
                    fn,
                    line_number
                );
		++number_of_errors;
		goto done;
	    }
	    return 0;
	}
	run_state_machine(c);
	switch (c)
	{
	case '\f':
	    ++pos;
	    break;

	case '\r':
	    c = getc(fp);
	    if (c == EOF)
	    {
		c = '\r';
		++unprintable;
		++pos;
		break;
	    }
	    if (c != '\n')
	    {
		ungetc(c, fp);
		++unprintable;
		++pos;
		++white_space;
		break;
	    }
	    dos_format = true;
	    // fall through...

	case '\n':
	    done:
	    if (unprintable && !unprintable_ok)
	    {
		fprintf
		(
                    stderr,
		    "%s: %d: line contains %d unprintable character%s\n",
		    fn,
		    line_number,
		    unprintable,
		    (unprintable == 1 ? "" : "s")
		);
		++number_of_errors;
	    }
	    if (white_space)
	    {
		fprintf
		(
                    stderr,
		    "%s: %d: white space at end of line\n",
		    fn,
		    line_number
		);
		++number_of_errors;
	    }
	    if (pos > limit && line_contains_white_space)
	    {
		fprintf
		(
                    stderr,
		    "%s: %d: line too long (by %d)\n",
		    fn,
		    line_number,
		    pos - limit
		);
		++number_of_errors;
	    }
	    if (pos)
		number_of_blank_lines = 0;
	    else
		++number_of_blank_lines;
	    return 1;

	case '\t':
	    pos = (pos + 8) & ~7;
	    ++white_space;
	    ++line_contains_white_space;
	    break;

	case ' ':
	    ++pos;
	    ++white_space;
	    ++line_contains_white_space;
	    break;

	default:
	    if (c == 0)
		binary_format = true;
	    assert(c != EOF);
	    if (!isprint((unsigned char)c))
		++unprintable;
	    ++pos;
	    white_space = 0;
	    break;
	}
    }
}


static bool
begins_with(const char *haystack, const char *needle)
{
    size_t len1 = strlen(haystack);
    size_t len2 = strlen(needle);
    return (len1 >= len2 && 0 == memcmp(haystack, needle, len2));
}


static bool
ends_with(const char *haystack, const char *needle)
{
    size_t len1 = strlen(haystack);
    size_t len2 = strlen(needle);
    return (len1 >= len2 && 0 == memcmp(haystack + len1 - len2, needle, len2));
}


void
check(const char *file_name)
{
    //
    // Skip over leading baseline symlinks.
    //
    const char *short_file_name = file_name;
    while (short_file_name[0] == 'b' && short_file_name[1] == 'l')
	short_file_name += 2;
    if (*short_file_name == '/')
	++short_file_name;
    else
	short_file_name = file_name;

    limit = 80;
    isa_c_file = ends_with(file_name, ".c") || ends_with(file_name, ".C");
    isa_cxx_file =
	ends_with(file_name, ".cc") || ends_with(file_name, ".CC") ||
	ends_with(file_name, ".cpp") || ends_with(file_name, ".CPP");
    isa_h_file = ends_with(file_name, ".h") || ends_with(file_name, ".H");
    unprintable_ok =
	(
	    begins_with(short_file_name, "lib/")
	&&
	    strstr(short_file_name, "/LC_MESSAGES/")
	);
    if (begins_with(short_file_name, "test/") && ends_with(file_name, ".sh"))
    {
	limit = 510;
	unprintable_ok = true;
    }
    if (ends_with(file_name, ".xml"))
    {
	limit = 510;
    }

    state = state_normal;
    fn = file_name;
    fp = fopen(fn, "r");
    if (!fp)
    	nfatal("open %s", fn);
    number_of_errors = 0;
    number_of_blank_lines = 0;
    line_number = 0;
    dos_format = false;
    binary_format = false;
    while (check_one_line())
	;
    if (dos_format)
    {
	fprintf(stderr, "%s: file in DOS format (must use UNIX format)\n", fn);
	++number_of_errors;
    }
    if (number_of_blank_lines > 0)
    {
	fprintf
	(
            stderr,
	    "%s: %d: found %d blank line%s at the end of the file\n",
	    fn,
            (line_number <= 1 ? 1 : line_number - 1),
	    number_of_blank_lines,
	    (number_of_blank_lines == 1 ? "" : "s")
	);
	++number_of_errors;
    }
    if (binary_format)
    {
	fprintf
	(
            stderr,
            "%s: file appears to be binary, it needs to be replaced "
                "with a plain-text file\n",
	    fn
	);
	++number_of_errors;
    }
    if (number_of_errors > 0 && !warning)
    {
	fatal_raw
	(
	    "%s: found %d fatal error%s",
	    fn,
	    number_of_errors,
	    (number_of_errors == 1 ? "" : "s")
	);
    }
    if (fclose(fp))
	nfatal("close %s", fn);
    fn = 0;
    fp = 0;
}
