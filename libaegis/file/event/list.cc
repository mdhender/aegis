//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>


file_event_list::~file_event_list()
{
    clear();
    delete [] item;
    item = 0;
    maximum = 0;
}


file_event_list::file_event_list() :
    length(0),
    maximum(0),
    item(0)
{
}


file_event_list::pointer
file_event_list::create()
{
    return pointer(new file_event_list());
}


file_event_list::file_event_list(const file_event_list &arg) :
    length(0),
    maximum(0),
    item(0)
{
    push_back(arg);
}


file_event_list::pointer
file_event_list::create(const pointer &arg)
{
    return pointer(new file_event_list(*arg));
}


file_event_list &
file_event_list::operator=(const file_event_list &arg)
{
    if (this != &arg)
    {
	clear();
	push_back(arg);
    }
    return *this;
}


void
file_event_list::clear()
{
    while (length)
    {
	--length;
	delete item[length];
	item[length] = 0;
    }
}


void
file_event_list::push_back(const file_event_list &arg)
{
    for (size_t j = 0; j < arg.size(); ++j)
    {
	push_back(new file_event(*arg[j]));
    }
}


void
file_event_list::push_back(file_event *fep)
{
    //
    // The event should be the last on the list.  If it isn't,
    // then a child branch supercedes the sequence.
    //
    trace(("file_event_list::push_back(this = %08lX, fep = %08lX)\n{\n",
	(long)this, (long)fep));
    while (length > 0 && item[length - 1]->get_when() >= fep->get_when())
    {
	length--;
	delete item[length];
	item[length] = 0;
    }

    //
    // Drop the event onto the end of the list.
    //
    if (length >= maximum)
    {
	size_t new_maximum = maximum * 2 + 4;
	file_event **new_item = new file_event * [new_maximum];
	for (size_t j = 0; j < length; ++j)
	    new_item[j] = item[j];
	delete [] item;
	item = new_item;
	maximum = new_maximum;
    }
    item[length++] = fep;
    trace(("}\n"));
}
