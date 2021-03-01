//
// aegis - project change supervisor
// Copyright (C) 2008, 2010, 2012 Peter Miller
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

#include <libaegis/change/file.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/comments.h>


validation_files_comments::~validation_files_comments()
{
}


validation_files_comments::validation_files_comments()
{
}


validation::pointer
validation_files_comments::create(void)
{
    return pointer(new validation_files_comments());
}


bool
validation_files_comments::check_branches()
    const
{
    return false;
}


bool
validation_files_comments::check_downloaded()
    const
{
    return false;
}


bool
validation_files_comments::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_comments::check_binaries()
    const
{
    return false;
}


bool
validation_files_comments::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    //
    // figure out what sort of file we are looking at
    //
    nstring base = path.basename();
    nstring base_ = base.downcase();
    bool isa_c_file = base.ends_with(".c");
    bool isa_cxx_file =
        base.ends_with(".C") ||
        base_.ends_with(".c++") ||
        base_.ends_with(".cc") ||
        base_.ends_with(".cpp") ||
        base.ends_with(".H") ||
        base_.ends_with(".h++") ||
        base_.ends_with(".hh") ||
        base_.ends_with(".hpp")
        ;
    bool isa_h_file =
        base_.ends_with(".h")
        ;
    if (!isa_c_file && !isa_cxx_file && !isa_h_file)
        return true;

    os_become_orig();
    bool ok = true;
    input ip = input_file_text_open(path);

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

    state_t state = state_normal;
    int line_number = 1;
    for (;;)
    {
        int ic = ip->getch();
        if (ic < 0)
            break;
        unsigned char c = ic;

        if (c == '\n')
            ++line_number;
        switch (state)
        {
        case state_normal:
            //
            // This state is for the body of a C or C++ file.  We aren't in
            // a string, or a character constant, or any kind of comment.
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
            // of a C or C++ comment, or just be a division operator.
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
                    isa_h_file = 0;
                    isa_cxx_file = 1;
                }
                if (isa_c_file)
                {
                    sub_context_ty sc;
                    sc.var_set_string("File_Name", src->file_name);
                    sc.var_set_long("Line_Number", line_number);
                    change_error
                    (
                        cp,
                        &sc,
                        i18n("$filename: $linenumber: C++ comment in a C file")
                    );
                    ok = false;
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
            // We have seen '/', '/', and possibly some content.
            // We are waiting for the newline which finishes the comment.
            //
            if (c == '\n')
                state = state_normal;
            break;

        case state_c_comment_begin:
            //
            // We have seen '/' and '*'.  We are waiting for '*' which could
            // start a Doxygen comment, or anything else which indicates
            // the start of a normal C comment.
            //
            if (c == '*')
                state = state_c_comment_doxygen;
            else
            {
                state = state_c_comment;
                goto check_c_comment;
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
                state = state_normal;
                goto check_c_comment;

            case '*':
                //
                // This is the start of a very ugly normal C comment.
                //
                state = state_c_comment_star;
                check_c_comment:
                if (isa_h_file && !isa_c_file)
                {
                    isa_h_file = false;
                    isa_c_file = true;
                }
                if (isa_cxx_file)
                {
                    sub_context_ty sc;
                    sc.var_set_string("File_Name", src->file_name);
                    sc.var_set_long("Line_Number", line_number);
                    change_error
                    (
                        cp,
                        &sc,
                        i18n("$filename: $linenumber: C comment in a C++ file")
                    );
                    ok = false;
                }
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
            // We have seen a '*' which could preceed a '/' to finish a
            // C comment.
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

    ip.close();
    os_become_undo();
    return ok;
}


// vim: set ts=8 sw=4 et :
