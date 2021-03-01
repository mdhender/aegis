//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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

#include <aegrep/arglex3.h>


static arglex_table_ty table[] =
{
    { "-After_Context", arglex_token_after_context },
    { "-Basic_regexp", arglex_token_basic_regexp },
    { "-Before_Context", arglex_token_before_context },
    { "-Binary", arglex_token_binary },
    { "-Binary_Files", arglex_token_binary_files },
    { "-Byte_offset", arglex_token_byte_offset },
    { "-COLor", arglex_token_color },
    { "-CONtext", arglex_token_context },
    { "-COUnt", arglex_token_count },
    { "-DEVices", arglex_token_devices },
    { "-DIRectories", arglex_token_directories },
    { "-EXclude", arglex_token_exclude },
    { "-Exclude_Directory", arglex_token_exclude_dir },
    { "-Exclude_From", arglex_token_exclude_from },
    { "-Expression", arglex_token_regexp },
    { "-Extended_Regexp", arglex_token_extended_regexp },
    { "-Files_WithOut_Matches", arglex_token_files_without_matches },
    { "-Files_With_Matches", arglex_token_files_with_matches },
    { "-Fixed_strings", arglex_token_fixed_strings },
    { "-INclude", arglex_token_include },
    { "-IGnore_Case", arglex_token_ignore_case },
    { "-Initial_Tab", arglex_token_initial_tab },
    { "-Invert_Match", arglex_token_invert_match },
    { "-Line_Buffered", arglex_token_line_buffered },
    { "-Line_Number", arglex_token_line_number },
    { "-Line_regexp", arglex_token_line_regexp },
    { "-Maximum_Count", arglex_token_maximum_count },
    { "-Memory_Map", arglex_token_memory_map },
    { "-No_Filename", arglex_token_no_filename },
    { "-No_Messages", arglex_token_no_messages },
    { "-Null", arglex_token_null },
    { "-Null_Data", arglex_token_null_data },
    { "-Only_matching", arglex_token_only_matching },
    { "-Perl_regexp", arglex_token_perl_regexp },
    { "-Quiet", arglex_token_quiet },
    { "-REgexp", arglex_token_regexp },
    { "-Recursive", arglex_token_recursive },
    { "-Silent", arglex_token_quiet },
    { "-Text", arglex_token_text },
    { "-Unix_Byte_Offsets", arglex_token_unix_byte_offsets },
    { "-With_Filename", arglex_token_with_filename },
    { "-Word_regexp", arglex_token_word_regexp },
    ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
    arglex2_init3(argc, argv, table);
}
