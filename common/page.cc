//
// aegis - project change supervisor
// Copyright (C) 2003-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>
#include <common/ac/sys/ioctl.h>
#include <common/ac/termios.h>
#include <common/ac/unistd.h>

#include <common/arglex.h>
#include <common/debug.h>
#include <common/page.h>


#define MIN_PAGE_WIDTH 40
// #define MAX_PAGE_WIDTH in option.h
#define DEFAULT_PAGE_WIDTH 80
#define MIN_PAGE_LENGTH 10
#define MAX_PAGE_LENGTH 30000
#define DEFAULT_PAGE_LENGTH 24

static int      page_length;
static int      terminal_length;
static int      page_width;
static int      terminal_width;
static int      default_terminal_sizes_done;


static void
default_terminal_sizes(void)
{
    if (default_terminal_sizes_done)
        return;
    default_terminal_sizes_done = 1;

    //
    // Use the command line arguments, if given.
    //
    if (!terminal_width)
        terminal_width = page_width;
    if (!terminal_length)
        terminal_length = page_length;

    //
    // Use environment variables, if given.
    //
    if (!terminal_width)
    {
        char            *cp;

        cp = getenv("COLS");
        if (cp)
        {
            int             n;

            n = atoi(cp);
            if (n > 0)
            {
                if (n < MIN_PAGE_WIDTH)
                    n = MIN_PAGE_WIDTH;
                if (n > MAX_PAGE_WIDTH)
                    n = MAX_PAGE_WIDTH;
                terminal_width = n;
            }
        }
    }
    if (!terminal_length)
    {
        char            *cp;

        cp = getenv("LINES");
        if (cp)
        {
            int             n;

            n = atoi(cp);
            if (n > 0)
            {
                if (n < MIN_PAGE_LENGTH)
                    n = MIN_PAGE_LENGTH;
                if (n > MAX_PAGE_LENGTH)
                    n = MAX_PAGE_LENGTH;
                terminal_length = n;
            }
        }
    }

    //
    // Use stdout attributes, if available.
    //
#ifdef TIOCGWINSZ
    if (!terminal_width || !terminal_length)
    {
        struct winsize  window_size;

        if (ioctl(0, TIOCGWINSZ, &window_size) == 0)
        {
            if (!terminal_width && window_size.ws_col > 0)
            {
                terminal_width = window_size.ws_col;
                if (terminal_width < MIN_PAGE_WIDTH)
                    terminal_width = MIN_PAGE_WIDTH;
                if (terminal_width > MAX_PAGE_WIDTH)
                    terminal_width = MAX_PAGE_WIDTH;
            }
            if (!terminal_length && window_size.ws_row > 0)
            {
                terminal_length = window_size.ws_row;
                if (terminal_length < MIN_PAGE_LENGTH)
                    terminal_length = MIN_PAGE_LENGTH;
                if (terminal_length > MAX_PAGE_LENGTH)
                    terminal_length = MAX_PAGE_LENGTH;
            }
        }
    }
#endif

    //
    // Use defaults if all else fails.
    //
    if (!terminal_width)
        terminal_width = DEFAULT_PAGE_WIDTH;
    if (!terminal_length)
        terminal_length = DEFAULT_PAGE_LENGTH;

    //
    // Make sure that it is possible to figure out which page header
    // style to use.  The variables are to avoid a compiler warning;
    //
#ifdef DEBUG
    {
        int a = DEFAULT_PAGE_LENGTH;
        int b = DEFAULT_PRINTER_LENGTH;
        assert(a < b / 2);
    }
#endif
}


int
page_width_set(int n)
{
    if (page_width)
        return PAGE_SET_ERROR_DUPLICATE;
    if (n < MIN_PAGE_WIDTH || n > MAX_PAGE_WIDTH)
        return PAGE_SET_ERROR_RANGE;
    page_width = n;
    return PAGE_SET_ERROR_NONE;
}


int
page_width_get(int dflt)
{
    //
    // must not generate a fatal error in this function,
    // as it is used by 'error.c' when reporting fatal errors.
    //
    // must not put tracing in this function,
    // because 'trace.c' uses it to determine the width.
    //
    if (dflt > 0)
    {
        //
        // At the moment, everything except output to files gives
        // a dflt argument of -1, meaning "to the terminal".
        // The default_terminal_sizes() function is for the
        // terminal *only*, so don't use if if we are given a
        // positive argument, meaning output to a file.
        //
        return (page_width > 0 ? page_width : dflt);
    }
    default_terminal_sizes();
    return terminal_width;
}


int
page_length_set(int n)
{
    if (page_length)
        return PAGE_SET_ERROR_DUPLICATE;
    if (n < MIN_PAGE_LENGTH || n > MAX_PAGE_LENGTH)
        return PAGE_SET_ERROR_RANGE;
    page_length = n;
    return PAGE_SET_ERROR_NONE;
}


int
page_length_get(int dflt)
{
    if (dflt > 0)
    {
        //
        // At the moment, everything except output to files gives
        // a dflt argument of -1, meaning "to the terminal".
        // The default_terminal_sizes() function is for the
        // terminal *only*, so don't use if if we are given a
        // positive argument, meaning output to a file.
        //
        return (page_length > 0 ? page_length : dflt);
    }
    default_terminal_sizes();
    return terminal_length;
}


// vim: set ts=8 sw=4 et :
