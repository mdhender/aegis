//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008 Peter Miller
//      Copyright (C) 2005 Matthew Lee;
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

#ifndef LIBAEGIS_RSS_FEED_H
#define LIBAEGIS_RSS_FEED_H

#include <common/nstring.h>

// Forward declarations
class rss_item;
struct project_ty;

/**
  * The rss_feed class is used to represent an RSS feed.
  *
  * Is is assumed that the client using this class has confirmed that
  * the project and change passed in are valid and exist.
  */
class rss_feed
{
public:
    /**
      * The destructor.
      */
    virtual ~rss_feed();

    /**
      * Constructor.
      *
      * @param the_project
      *   A pointer to a project.
      * @param the_change
      *   A pointer to a change.
      * @param the_file
      *   A pointer to the filename associated with this feed.
      */
    rss_feed(project_ty *the_project, change::pointer the_change,
	const nstring &the_file);

    /**
      * Add an item to the RSS feed.
      *
      * @param the_item
      *   The new item to be added to the feed.
      */
    void push_back(rss_item *the_item);

    /**
      * The back method is used to obtain the last item on the list.
      */
    rss_item *back() const;
    rss_item *pop_back();

    /**
      * The parse method is used to read and parse the RSS feed file
      * into this object.  All items read from the file are appended to
      * the feed, via the push_back method.  If the file does not exits,
      * this method does nothing.
      */
    void parse();

    /**
      * The print method is used to print the contents of the feed to
      * the file named in the constructor.
      */
    void print() const;

    /**
      * The print method is used to print the contents of the feed to
      * the file named in the constructor.
      *
      * @param op
      *     The output stream in which to write the output.
      */
    void print(output::pointer op) const;

    void handle_item();
    void handle_item_title(const nstring &value);
    void handle_item_description(const nstring &value);
    void handle_item_pub_date(const nstring &value);
    void handle_item_link(const nstring &value);
    void handle_item_author(const nstring &value);
    void handle_item_category(const nstring &value);
    void handle_item_comments(const nstring &value);
    void handle_item_enclosure(const nstring &value);
    void handle_item_guid(const nstring &value, bool attr);
    void handle_item_source(const nstring &value);

    size_t size() const { return item_count; }

    /**
      * The channel_elements_from_project method is used to set the
      * channel's descriptive elements from the given project.
      */
    void channel_elements_from_project();

    /**
      * The channel_elements_from_change method is used to set the
      * craete a new feed item, and set its descriptive elements from
      * the given change.
      */
    void channel_elements_from_change();

    void title_set(const nstring &rg);
    void description_set(const nstring &rg);

private:
    /**
      * The project with which this RSS feed is associated.
      */
    project_ty *project;

    /**
      * The change with which this RSS feed is associated.
      */
    change::pointer cp;

    /**
      * The filename with which this RSS feed is associated.
      */
    nstring filename;

    /**
      * The title instance variable is used to remember
      * "The name of the channel.  It's how people refer to your
      * service.  If you have an HTML website that contains the same
      * information as your RSS file, the title of your channel should
      * be the same as the title of your website."
      */
    nstring title;

    /**
      * The description instance variable is used to remember a
      * "Phrase or sentence describing the channel."
      */
    nstring description;

    /**
      * The language instance variable is used to remember
      * "The language the channel is written in."
      */
    nstring language;

    /**
      * The pub_date instance variable is used to remember
      * "The publication date for the content in the channel.  For
      * example, the New York Times publishes on a daily basis, the
      * publication date flips once every 24 hours.  That's when the
      * pubDate of the channel changes."
      */
    nstring pub_date;

    /**
      * The last_build_date instance variable is used to remember
      * "The last time the content of the channel changed."
      */
    nstring last_build_date;

    /**
      * The generator instance variable is used to remember
      * "A string indicating the program used to generate the channel."
      */
    nstring generator;

    /**
      * The docs instance variable is used to remember
      * "A URL that points to the documentation for the format used in
      * the RSS file. It's probably a pointer to this page. It's for
      * people who might stumble across an RSS file on a Web server 25
      * years from now and wonder what it is."
      */
    nstring docs;

    /**
      * The link instance variable is used to remember
      * "The URL to the HTML website corresponding to the channel."
      */
    nstring link;

    rss_item **item;
    size_t item_count;
    size_t item_max;

    /**
      * The default constructor.  Don't use.
      */
    rss_feed();

    /**
      * The copy constructor.  Do not use.
      */
    rss_feed(const rss_feed &arg);

    /**
      * The assignment operator.  Do not use.
      */
    rss_feed &operator=(const rss_feed &arg);
};

#endif // LIBAEGIS_RSS_FEED_H
