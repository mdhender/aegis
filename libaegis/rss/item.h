//
//      aegis - project change supervisor
//      Copyright (C) 2005 Matthew Lee
//      Copyright (C) 2007, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_RSS_ITEM_H
#define LIBAEGIS_RSS_ITEM_H

#include <common/nstring.h>
#include <libaegis/change.h>

// Forward declarations
class output_ty;


/**
  * The rss_item class is used to represent an Item in an RSS feed.
  */
class rss_item
{
public:
    /**
      * The destructor.
      */
    virtual ~rss_item();

    /**
      * The default constructor.
      */
    rss_item();

    /**
      * The copy constructor.
      */
    rss_item(const rss_item &);

    /**
      * The assignment operator.
      */
    rss_item &operator=(const rss_item &);

    /**
      * The handle_title method is used to set the title of the item.
      */
    void handle_title(const nstring &value);

    /**
      * The handle_description method is used to set the description of
      * the item.
      */
    void handle_description(const nstring &value);

    /**
      * The handle_pub_date method is used to set the pub_date of the
      * item.
      */
    void handle_pub_date(const nstring &value);

    /**
      * Add a link (URL) for the item.
      */
    void handle_link(const nstring &value);

    /**
      * Add the email address of the author of the item.
      */
    void handle_author(const nstring &value);

    /**
      * Add a category for this item to be included with.
      */
    void handle_category(const nstring &value);

    /**
      * Add the URL of a page for comments relating to the item.
      */
    void handle_comments(const nstring &value);

    /**
      * Add a description of a media object attached to the item.
      */
    void handle_enclosure(const nstring &value);

    /**
      * Add a string that uniquely identifies the item.
      */
    void handle_guid(const nstring &value, bool is_perma_link);

    /**
      * Add the RSS channel that the item came from.
      */
    void handle_source(const nstring &value);

    /**
      * Print out the contents of the item.
      */
    void print(output::pointer out) const;

    /**
      * the handle_change method is used to set the attributes of the
      * RSS item from the given change.
      *
      * @param cp
      *     The change to take the attributes from.
      */
    void handle_change(change::pointer cp);

private:
    /**
      * The item's title.
      */
    nstring title;

    /**
      * The item's description.
      */
    nstring description;

    /**
      * The item's publication date.
      */
    nstring pub_date;

    /**
      * The item's link (URL).
      */
    nstring link;

    /**
      * The email address of the author of the item.
      */
    nstring author;

    /**
      * The category of the item.
      */
    nstring category;

    /**
      * URL of a page for comments relating to the item.
      */
    nstring comments;

    /**
      * Describes a media object that is attached to the item.
      */
    nstring enclosure;

    /**
      * A string that uniquely identifies the item.
      */
    nstring guid;

    /**
      * A flag indicating whether or not the GUID is a permanent link.
      */
    bool guidIsPermaLink;

    /**
      * The RSS channel that the item came from.
      */
    nstring source;
};

#endif // LIBAEGIS_RSS_ITEM_H
