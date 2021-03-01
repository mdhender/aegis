//
// aegis - project change supervisor
// Copyright (C) 1994-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/symtab.h>
#include <common/sizeof.h>
#include <libaegis/aer/func/capitalize.h>
#include <libaegis/aer/func/change.h>
#include <libaegis/aer/func/columns.h>
#include <libaegis/aer/func/count.h>
#include <libaegis/aer/func/dirname.h>
#include <libaegis/aer/func/downcase.h>
#include <libaegis/aer/func/eject.h>
#include <libaegis/aer/func/getenv.h>
#include <libaegis/aer/func/gettime.h>
#include <libaegis/aer/func/getuid.h>
#include <libaegis/aer/func/keys.h>
#include <libaegis/aer/func/length.h>
#include <libaegis/aer/func/mtime.h>
#include <libaegis/aer/func/need.h>
#include <libaegis/aer/func/now.h>
#include <libaegis/aer/func/page_width.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/func/project.h>
#include <libaegis/aer/func/quote.h>
#include <libaegis/aer/func/round.h>
#include <libaegis/aer/func/sort.h>
#include <libaegis/aer/func/split.h>
#include <libaegis/aer/func/sprintf.h>
#include <libaegis/aer/func/strftime.h>
#include <libaegis/aer/func/substitute.h>
#include <libaegis/aer/func/substr.h>
#include <libaegis/aer/func/terse.h>
#include <libaegis/aer/func/title.h>
#include <libaegis/aer/func/typeof.h>
#include <libaegis/aer/func/upcase.h>
#include <libaegis/aer/func/wrap.h>
#include <libaegis/aer/value/func.h>


rpt_func::~rpt_func()
{
}


rpt_func::rpt_func()
{
}


typedef rpt_func::pointer (*table_t)(void);

static table_t table[] =
{
    &rpt_func_basename::create,
    &rpt_func_capitalize::create,
    &rpt_func_ceil::create,
    &rpt_func_change_number::create,
    &rpt_func_change_number_set::create,
    &rpt_func_columns::create,
    &rpt_func_count::create,
    &rpt_func_dirname::create,
    &rpt_func_downcase::create,
    &rpt_func_eject::create,
    &rpt_func_floor::create,
    &rpt_func_getenv::create,
    &rpt_func_gettime::create,
    &rpt_func_getuid::create,
    &rpt_func_keys::create,
    &rpt_func_length::create,
    &rpt_func_mktime::create,
    &rpt_func_mtime::create,
    &rpt_func_need::create,
    &rpt_func_now::create,
    &rpt_func_page_length::create,
    &rpt_func_page_width::create,
    &rpt_func_print::create,
    &rpt_func_project_name::create,
    &rpt_func_project_name_set::create,
    &rpt_func_quote_html::create,
    &rpt_func_quote_tcl::create,
    &rpt_func_quote_url::create,
    &rpt_func_round::create,
    &rpt_func_sort::create,
    &rpt_func_split::create,
    &rpt_func_sprintf::create,
    &rpt_func_strftime::create,
    &rpt_func_substitute::create,
    &rpt_func_substr::create,
    &rpt_func_terse::create,
    &rpt_func_title::create,
    &rpt_func_trunc::create,
    &rpt_func_typeof::create,
    &rpt_func_unquote_url::create,
    &rpt_func_upcase::create,
    &rpt_func_working_days::create,
    &rpt_func_wrap::create,
    &rpt_func_wrap_html::create,
};


void
rpt_func::init(symtab<rpt_value::pointer> &symbol_table)
{
    for (size_t j = 0; j < SIZEOF(table); ++j)
    {
        rpt_func::pointer fp = table[j]();
        symbol_table.assign(fp->name(), rpt_value_func::create(fp));
    }
}


// vim: set ts=8 sw=4 et :
