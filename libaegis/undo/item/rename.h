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

#ifndef LIBAEGIS_UNDO_ITEM_RENAME_H
#define LIBAEGIS_UNDO_ITEM_RENAME_H

#include <common/nstring.h>
#include <libaegis/undo/item.h>

/**
  * The undo_item_rename class is used to represent a rename action to
  * be performed in the event of an error.
  *
  * It renames a file, moving it between directories if required.
  *
  * Any other hard links to the file (as created using link(2)) are
  * unaffected.
  *
  * If newpath already exists it will be atomically replaced (subject
  * to a few conditions), so that there is no point at which another
  * process attempting to access newpath will find it missing.
  *
  * If newpath exists but the operation fails for some reason rename
  * guarantees to leave an instance of newpath in place.
  *
  * However, when overwriting there will probably be a window in which
  * both oldpath and newpath refer to the file being renamed.
  *
  * If oldpath refers to a symbolic link the link is renamed; if newpath
  * refers to a symbolic link the link will be overwritten.
  */
class undo_item_rename:
    public undo_item
{
public:
    /**
      * The destructor.
      */
    virtual ~undo_item_rename();

    /**
      * The constructor.
      *
      * \param oldpath
      *     What the file is called now.
      * \param newpath
      *     What the file will be called then.
      */
    undo_item_rename(const nstring &oldpath, const nstring &newpath);

    /**
      * The equality operator is used to see if two rename items are
      * equal.
      */
    bool
    operator==(const undo_item_rename &rhs)
	const
    {
	return (oldpath == rhs.oldpath && newpath == rhs.newpath);
    }

    // See base class for documentation.
    void action();

    // See base class for documentation.
    void unfinished();

private:
    /**
      * The oldpath instance variable is used to remember what the file
      * is called now.
      */
    nstring oldpath;

    /**
      * The newpath instance variable is used to remember what the file
      * will be called then.
      */
    nstring newpath;

    /**
      * The default constructor.  Do not use.
      */
    undo_item_rename();

    /**
      * The copy constructor.  Do not use.
      */
    undo_item_rename(const undo_item_rename &);

    /**
      * The assignment operator.  Do not use.
      */
    undo_item_rename &operator=(const undo_item_rename &);
};

#endif // LIBAEGIS_UNDO_ITEM_RENAME_H
