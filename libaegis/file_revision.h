//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_FILE_REVISION_H
#define LIBAEGIS_FILE_REVISION_H

#include <common/nstring.h>

/**
  * The file_revision class is used to represent the path to a file
  * revision, and wether or not is needs to be unlinked by the
  * destructor.
  */
class file_revision
{
public:
    /**
      * The destructor.
      * Thou shalt not derive from this class.
      */
    ~file_revision();

    /**
      * The constructor.
      */
    file_revision(const nstring &filename, bool need_to_unlink);

    /**
      * The copy constructor.
      */
    file_revision(const file_revision &);

    /**
      * The assignment operator.
      */
    file_revision &operator=(const file_revision &);

    /**
      * The get_path method is used to extract the path of the file
      * containing the file revision.
      */
    nstring get_path() const { return ref->get_path(); }

private:
    class inner
    {
    public:
	/**
	  * The destructor.
	  * Thou shalt not derive from this class.
          *
          * This method is private because you are supposed to call
          * one_fewer() so that the instance will delete itself once the
          * last reference is gone.
	  */
	~inner();

	/**
	  * The constructor.
	  */
	inner(const nstring &filename, bool need_to_unlink);

	/**
	  * The get_path method is used to extract the path of the file
	  * containing the file revision.
	  */
	nstring get_path() const { return filename; }

	/**
          * The one_fewer method is used to notify this instance that
          * there is one fewer reference to it.  Once all references are
          * gone, this instance will delete itself.
	  */
	void one_fewer();

	/**
          * The one_more method is used to notify this instance that
          * there is one more reference to it.
	  */
	void one_more() { ++reference_count; }

    private:
	/**
          * The reference_count instance variable is used to remember
          * how many references exist to this.
	  */
	long reference_count;

	/**
	  * The filename instance variable is used to remember the absolute
	  * path of the file.
	  */
	nstring filename;

	/**
	  * The need_to_unlink instance variable is used to remember wether
	  * or not we need to unlink the file in the destructor.
	  */
	bool need_to_unlink;

	/**
	  * The default constructor.  Do not use.
	  */
	inner();

	/**
	  * The copy constructor.  Do not use.
	  */
	inner(const inner &);

	/**
	  * The assignment operator.  Do not use.
	  */
	inner &operator=(const inner &);
    };

    /**
      * The ref instance variable is used to remember the reference to
      * the inner reference-counted data.
      */
    inner *ref;

    /**
      * The default constructor.
      */
    file_revision();
};

#endif // LIBAEGIS_FILE_REVISION_H
