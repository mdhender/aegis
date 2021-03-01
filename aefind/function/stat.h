//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2005-2008 Peter Miller
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

#ifndef AEFIND_FUNCTION_STAT_H
#define AEFIND_FUNCTION_STAT_H

#include <aefind/tree/monadic.h>

class tree_list; // forward

/**
  * The tree_atime class is used to represent an expression tree which
  * evatuates to the time-last-accessed of the file argument.
  */
class tree_atime:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_atime();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_atime(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_atime();

    /**
      * The copy constructor.  Do not use.
      */
    tree_atime(const tree_atime &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_atime &operator=(const tree_atime &);
};


/**
  * The tree_ctime class is used to represent an expression tree which
  * evatuates to teh time-inode-last-modified of the file argument.
  */
class tree_ctime:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_ctime();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_ctime(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_ctime();

    /**
      * The copy constructor.  Do not use.
      */
    tree_ctime(const tree_ctime &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_ctime &operator=(const tree_ctime &);
};


/**
  * The tree_gid class is used to represent an expression tree which
  * evatuates to the group ID of the filename argument.
  */
class tree_gid:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_gid();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_gid(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_gid();

    /**
      * The copy constructor.  Do not use.
      */
    tree_gid(const tree_gid &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_gid &operator=(const tree_gid &);
};


/**
  * The tree_ino class is used to represent an expression tree which
  * evatuates to the inode number of the file argument.
  */
class tree_ino:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_ino();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_ino(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_ino();

    /**
      * The copy constructor.  Do not use.
      */
    tree_ino(const tree_ino &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_ino &operator=(const tree_ino &);
};


/**
  * The tree_mode class is used to represent an expression tree which
  * evatuates to the file permissions mode bits of the argument file.
  */
class tree_mode:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_mode();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_mode(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_mode();

    /**
      * The copy constructor.  Do not use.
      */
    tree_mode(const tree_mode &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_mode &operator=(const tree_mode &);
};


/**
  * The tree_mtime class is used to represent an expression tree which
  * evatuates to the last-time-modified of the file argument.
  */
class tree_mtime:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_mtime();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_mtime(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_mtime();

    /**
      * The copy constructor.  Do not use.
      */
    tree_mtime(const tree_mtime &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_mtime &operator=(const tree_mtime &);
};


/**
  * The tree_nlink class is used to represent an expression tree which
  * evatuates to the number of links to the filename argument.
  */
class tree_nlink:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_nlink();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_nlink(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_nlink();

    /**
      * The copy constructor.  Do not use.
      */
    tree_nlink(const tree_nlink &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_nlink &operator=(const tree_nlink &);
};


/**
  * The tree_size class is used to represent an expression tree which
  * evatuates to the size in bytes of the filename argument.
  */
class tree_size:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_size();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_size(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_size();

    /**
      * The copy constructor.  Do not use.
      */
    tree_size(const tree_size &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_size &operator=(const tree_size &);
};


/**
  * The tree_uid class is used to represent an expression tree which
  * evatuates to the user ID of the owner of the filename argument.
  */
class tree_uid:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_uid();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_uid(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_uid();

    /**
      * The copy constructor.  Do not use.
      */
    tree_uid(const tree_uid &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_uid &operator=(const tree_uid &);
};


/**
  * The tree_type class is used to represent an expression tree which
  * evatuates to the name of the type of the filename argument.
  */
class tree_type:
    public tree_monadic
{
public:
    /**
      * The destructor.
      */
    virtual ~tree_type();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * clas smethod instead.
      */
    tree_type(const pointer &arg);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param arg
      *     The singel argument to this function.
      */
    static pointer create(const pointer &arg);

    /**
      * The create_l class method is used to create new dynamically
      * allocated instance of this class.
      *
      * @param args
      *     The arguments to this function.
      */
    static pointer create_l(const tree_list &arg);

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate(string_ty *, string_ty *, string_ty *,
        struct stat *) const;

    // See base class for documentation.
    tree::pointer optimize() const;

private:
    /**
      * The default constructor.  Do not use.
      */
    tree_type();

    /**
      * The copy constructor.  Do not use.
      */
    tree_type(const tree_type &);

    /**
      * The assignment operator.  Do not use.
      */
    tree_type &operator=(const tree_type &);
};

#endif // AEFIND_FUNCTION_STAT_H
