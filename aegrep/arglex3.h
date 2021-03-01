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

#ifndef AEGREP_ARGLEX3_H
#define AEGREP_ARGLEX3_H

#include <libaegis/arglex2.h>

enum
{
    arglex_token_after_context = ARGLEX2_MAX,
    arglex_token_basic_regexp,
    arglex_token_before_context,
    arglex_token_binary,
    arglex_token_binary_files,
    arglex_token_byte_offset,
    arglex_token_color,
    arglex_token_context,
    arglex_token_count,
    arglex_token_devices,
    arglex_token_directories,
    arglex_token_exclude,
    arglex_token_exclude_dir,
    arglex_token_exclude_from,
    arglex_token_extended_regexp,
    arglex_token_files_with_matches,
    arglex_token_files_without_matches,
    arglex_token_fixed_strings,
    arglex_token_ignore_case,
    arglex_token_include,
    arglex_token_initial_tab,
    arglex_token_invert_match,
    arglex_token_line_buffered,
    arglex_token_line_number,
    arglex_token_line_regexp,
    arglex_token_maximum_count,
    arglex_token_memory_map,
    arglex_token_no_filename,
    arglex_token_no_messages,
    arglex_token_null,
    arglex_token_null_data,
    arglex_token_only_matching,
    arglex_token_perl_regexp,
    arglex_token_quiet,
    arglex_token_recursive,
    arglex_token_regexp,
    arglex_token_text,
    arglex_token_unix_byte_offsets,
    arglex_token_with_filename,
    arglex_token_word_regexp,
    ARGLEX3_MAX
};

void arglex3_init(int argc, char **argv);

#endif // AEGREP_ARGLEX3_H
