//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#include <aemakegen/flavour.h>
#include <aemakegen/process_item/autoconf.h>
#include <aemakegen/process_item/c.h>
#include <aemakegen/process_item/config_ac_in.h>
#include <aemakegen/process_item/cxx.h>
#include <aemakegen/process_item/datadir.h>
#include <aemakegen/process_item/datarootdir.h>
#include <aemakegen/process_item/etc_man.h>
#include <aemakegen/process_item/etc_msgfm_sh.h>
#include <aemakegen/process_item/i18n.h>
#include <aemakegen/process_item/lex.h>
#include <aemakegen/process_item/libdir.h>
#include <aemakegen/process_item/library.h>
#include <aemakegen/process_item/man_page.h>
#include <aemakegen/process_item/pkgconfig.h>
#include <aemakegen/process_item/scripts.h>
#include <aemakegen/process_item/test_sh.h>
#include <aemakegen/process_item/yacc.h>
#include <aemakegen/target.h>


flavour::~flavour()
{
}


flavour::flavour(target &a_tgt) :
    tgt(a_tgt)
{
}


void
flavour::register_process(const process_item::pointer &proc)
{
    tgt.register_process(proc);
}


void
flavour::register_process_front(const process_item::pointer &proc)
{
    tgt.register_process_front(proc);
}


void
flavour::set_process(void)
{
    register_process(process_item_configure_ac_in::create(tgt));
    register_process(process_item_autoconf::create(tgt));
    register_process(process_item_yacc::create(tgt));
    register_process(process_item_lex::create(tgt));
    register_process(process_item_cxx::create(tgt));
    register_process(process_item_c::create(tgt));
    register_process(process_item_scripts::create(tgt));
    register_process(process_item_etc_msgfmt_sh::create(tgt));
    register_process(process_item_i18n::create(tgt));
    register_process(process_item_test_sh::create(tgt));

    // Must precede both datadir and libdir items.
    register_process(process_item_pkgconfig::create(tgt));

    register_process(process_item_datadir::create(tgt));
    register_process(process_item_datarootdir::create(tgt));
    register_process(process_item_libdir::create(tgt));
    register_process(process_item_man_page::create(tgt));
    register_process(process_item_etc_man::create(tgt));
    register_process(process_item_library::create(tgt));
}


// vim: set ts=8 sw=4 et :
