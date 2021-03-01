//
//	aegis - project change supervisor
//	Copyright (C) 1992-2006, 2008 Peter Miller
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

#ifndef OUTPUT_PAGER_H
#define OUTPUT_PAGER_H

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_pager class is used to represent an output stream
  * which is run thropugh a paginator onto the users terminal.
  * This is controlled by the PAGER environment variable.
  */
class output_pager:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_pager();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * open class method instead.
      */
    output_pager();

public:
    /**
      * The set class method is used to
      *
      * \param flag
      *     The command line flag: 0 means --no-pager, 1 means --pager
      * \param usage
      *     The function to call toi print a command line usage message
      *     and exit.
      */
    static void set(int flag, void(*usage)(void));

    /**
      * The open class method is used to create a paginated output
      * stream if the output is to a terminal, and if the user has not
      * prevented it with a command line option.
      */
    static pointer open(void);

    /**
      * The cleanup class method is called at quit() time to cleanup any
      * output pager than may still be running.  This function shall
      * only be called by the quit_action_pager class.
      */
    static void cleanup(void);

protected:
    // See base class for documentation.
    nstring filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    void flush_inner();

private:
    void *vdeeper;
    int pid;
    nstring pager;
    bool bol;

    void pipe_open();
    void pager_error();

    /**
      * The copy constructor.  Do not use.
      */
    output_pager(const output_pager &);

    /**
      * The assignment operator.  Do not use.
      */
    output_pager &operator=(const output_pager &);
};

/**
  * The option_pager_set function is used to
  *
  * \param flag
  *     The command line flag: 0 means --no-pager, 1 means --pager
  * \param usage
  *     The function to call toi print a command line usage message and exit.
  */
void option_pager_set(int flag, void(*usage)(void));

#endif // OUTPUT_PAGER_H
