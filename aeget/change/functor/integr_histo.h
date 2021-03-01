//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#ifndef AEGET_CHANGE_FUNCTOR_INTEGR_HISTO_H
#define AEGET_CHANGE_FUNCTOR_INTEGR_HISTO_H

#include <libaegis/change/functor.h>
#include <common/symtab.h>

class project; // forward

/**
  * The change_functor_integration_histogram class is used to represent
  * the data collected by walking the project change tree and reporting
  * it as a histogram.
  */
class change_functor_integration_histogram:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_integration_histogram();

    /**
      * The constructor.
      */
    change_functor_integration_histogram(project_ty *pp, time_t earliest,
	time_t latest, bool recursive, const char *format);

    // See base class for documentation.
    time_t earliest();

    // See base class for documentation.
    time_t latest();

    // See base class for documentation.
    bool recurse_branches();

    // See base class for documentation.
    void operator()(change::pointer cp);

    /**
      * Print the histogram as HTML to the standard output.
      */
    void print();

private:
    project_ty *pp;
    time_t min_time;
    time_t max_time;
    bool recurse;
    const char *format;
    long size_of_biggest_bucket;
    time_t min_seen;
    time_t max_seen;
    symtab_ty bucket;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_integration_histogram();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_integration_histogram(
	const change_functor_integration_histogram &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_integration_histogram &operator=(
	const change_functor_integration_histogram &);
};

#endif // AEGET_CHANGE_FUNCTOR_INTEGR_HISTO_H
