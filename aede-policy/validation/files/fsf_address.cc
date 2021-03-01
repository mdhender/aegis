//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/change/file.h>
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/fsf_address.h>


//
// These patterns must NOT match themselves.
// This is why "Free" is broken up.
//
// Be mindful of the commas between the pattern strings.
//
static const char *old_addresses[] =
{
    "F" "ree Software Foundation Inc., "
    "59 Temple Place",

    "F" "ree Software Foundation Inc., "
    "675 Mass Ave",

    "F" "ree Software Foundation Inc., "
    "675 Massachusetts Ave",

    "F" "ree Software Foundation Inc., "
    "675 Mass Avenue",

    "F" "ree Software Foundation Inc., "
    "675 Massachusetts Avenue",
};

//
// This is the current address.
// In time I expect this will become yet another "old" address.
//
static const char current_address[] =
    "Free Software Foundation, Inc., "
    "51 Franklin Street, Fifth Floor, "
    "Boston, MA 02110-1301 USA.";


validation_files_fsf_address::~validation_files_fsf_address()
{
}


validation_files_fsf_address::validation_files_fsf_address() :
    validation_files()
{
}


/**
  * The match function is used to match a buffer full of text against an
  * address pattern.
  * Runs of !isalnum characters in the pattern match
  * runs of !isalnum characters in the text.
  *
  * @param buffer
  *     The text to be matched to the pattern.
  * @param nbytes
  *     The length of the text buffer.
  * @param pattern
  *     The pattern to be matched to.  NUL terminated C string.  Runs
  *     of !isalnum characters in the pattern match runs of !isalnum
  *     characters in the text; isalnum characters are literals.
  *     Pattern must start with literal.
  * @returns
  *     0 for no match, or the line number within the buffer of the
  *     match
  */

static int
match(const char *buffer, size_t nbytes, const char *pattern)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    const char *begin = buffer;
    assert(pattern[0] != ' ');
    size_t pattern_nbytes = strlen(pattern);
    const char *pattern_end = pattern + pattern_nbytes;

    //
    // Find length of first pattern word.
    // We will use the first word to eliminate most false positives.
    //
    size_t pattern_word_len = 1;
    while
    (
        pattern_word_len < pattern_nbytes
    &&
        isalnum((unsigned char)pattern[pattern_word_len])
    )
        ++pattern_word_len;
    trace(("pattern_word = \"%.*s\"\n", (int)pattern_word_len, pattern));

    const char *buffer_end = buffer + nbytes;
    while (buffer < buffer_end)
    {
        trace(("buffer = %p", buffer));
        const char *w =
            (const char *)
            memmem(buffer, buffer_end - buffer, pattern, pattern_word_len);
        if (!w)
            break;
        trace(("w = %p\n", w));

        const char *buf_p = w + pattern_word_len;
        const char *pat_p = pattern + pattern_word_len;
        bool is_a_match = true;
        while (pat_p < pattern_end)
        {
            if (buf_p >= buffer_end)
            {
                is_a_match = false;
                break;
            }

            unsigned char pc = *pat_p++;
            trace(("pc = '%c'\n", pc));
            unsigned char bc = *buf_p++;
            trace(("bc = '%c'\n", bc));
            if (!isalnum(pc))
            {
                if (isalnum(bc))
                {
                    is_a_match = false;
                    break;
                }
                while (pat_p < pattern_end && !isalnum((unsigned char)*pat_p))
                    ++pat_p;
                for (;;)
                {
                    if (buf_p >= buffer_end)
                        break;
                    bc = *buf_p;
                    if
                    (
                        bc == '.'
                    &&
                        buf_p + 3 <= buffer_end
                    &&
                        buf_p[1] == 'b'
                    &&
                        buf_p[2] == 'r'
                    )
                    {
                        // assume this is a groff ".br" command
                        // to separate address lines
                        buf_p += 3;
                        continue;
                    }
                    if
                    (
                        bc == '<'
                    &&
                        buf_p + 4 <= buffer_end
                    &&
                        (buf_p[1] == 'b' || buf_p[1] == 'B')
                    &&
                        (buf_p[2] == 'r' || buf_p[2] == 'R')
                    &&
                        buf_p[3] == '>'
                    )
                    {
                        // assume this is a HTML "<br>" element
                        // to separate address lines
                        buf_p += 4;
                        continue;
                    }
                    if
                    (
                        bc == 'd'
                    &&
                        buf_p + 3 <= buffer_end
                    &&
                        buf_p[1] == 'n'
                    &&
                        buf_p[2] == 'l'
                    )
                    {
                        // assume this is an M4 comment (like
                        // configure.ac uses).
                        buf_p += 3;
                        continue;
                    }
                    if (isalnum(bc))
                        break;
                    ++buf_p;
                }
            }
            else
            {
                if (isupper(pc))
                    pc = tolower(pc);
                if (isupper(bc))
                    bc = tolower(bc);
                if (pc != bc)
                {
                    is_a_match = false;
                    break;
                }
            }
        }
        if (is_a_match)
        {
            //
            // figure out line number
            //
            int line_number = 1;
            for (const char *cp = begin; cp < w; ++cp)
                if (*cp == '\n')
                    ++line_number;
            return line_number;
        }

        buffer = w + 1;
    }
    return 0;
}


bool
validation_files_fsf_address::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
        return true;

    //
    // Read the first few kB of the file.
    //
    os_become_orig();
    input ip = input_file_open(path);
    char buffer[1 << 14];
    size_t nbytes = ip->read(buffer, sizeof(buffer));
    ip.close();
    os_become_undo();

    //
    // Check against each of the old addresses.
    //
    for
    (
        const char **pattern_ptr = old_addresses;
        pattern_ptr < ENDOF(old_addresses);
        ++pattern_ptr
    )
    {
        int line_number = match(buffer, nbytes, *pattern_ptr);
        if (line_number > 0)
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            sc.var_set_charstar("Suggest", current_address);
            sc.var_optional("Suggest");
            change_error(cp, &sc, i18n("$filename: old FSF address"));
            return false;
        }
    }

    return true;
}
