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

#include <aemakegen/process_queu.h>


process_queue::~process_queue()
{
}


process_queue::process_queue()
{
}


void
process_queue::register_process_item(const process_item::pointer &pip)
{
    queue.push_back(pip);
}


void
process_queue::register_process_item_front(const process_item::pointer &pip)
{
    queue.push_front(pip);
}


void
process_queue::register_default(const process_item::pointer &pip)
{
    dflt = pip;
}


// ----------  preprocess  -------------------------------------------------


void
process_queue::run_preprocess(const nstring_list &filenames)
{
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring fn = filenames[j];
        run_preprocess(fn);
    }

    // see if any want one last squeak
    for (queue_t::iterator it = queue.begin(); it != queue.end(); ++it)
    {
        process_item::pointer &pip = *it;
        pip->preprocess_end();
    }
}


void
process_queue::run_preprocess(const nstring &filename)
{
    for (queue_t::iterator it = queue.begin(); it != queue.end(); ++it)
    {
        process_item::pointer &pip = *it;
        if (pip->run_preprocess(filename))
            return;
    }

    // not matched elsewhere
    if (dflt)
        dflt->run_preprocess(filename);
}


// ----------  process  ----------------------------------------------------


void
process_queue::run_process(const nstring_list &filenames)
{
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring fn = filenames[j];
#if 0
        fprintf(stderr, "%s: %d: filename = %s\n", __FILE__, __LINE__,
            fn.quote_c().c_str());
#endif
        run_process(fn);
    }

#if 0
    // see if any want one last squeak
    for (queue_t::iterator it = queue.begin(); it != queue.end(); ++it)
    {
        process_item::pointer &pip = *it;
        pip->process_end();
    }
#endif
}


void
process_queue::run_process(const nstring &filename)
{
    for (queue_t::iterator it = queue.begin(); it != queue.end(); ++it)
    {
        process_item::pointer &pip = *it;
        if (pip->run_process(filename))
            return;
    }

    // not matched elsewhere
    if (dflt)
        dflt->run_process(filename);
}


// vim: set ts=8 sw=4 et :
