//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2007, 2008 Walter Franzini
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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/errno.h>
#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/now.h>
#include <common/nstring.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/uuidentifier.h>
#include <common/uuidentifier/translate.h>
#include <common/version_stmp.h>
#include <libaegis/change.h>
#include <libaegis/emit/project.h>
#include <libaegis/http.h>
#include <libaegis/os.h>
#include <libaegis/project.h>


bool http_fatal_noerror;


void
http_fatal(http_error_t oops, const char *fmt, ...)
{
    va_list         ap;

    va_start(ap, fmt);
    if (oops != http_error_ok && !http_fatal_noerror)
        printf("Status: %d Error\n", oops);
    printf("Content-Type: text/html\n\n");
    printf("<html><head><title>Error</title></head><body><h1>Error</h1>\n");
    printf("The request failed because:\n<em>");
    vprintf(fmt, ap);
    va_end(ap);
    printf("</em>\n</body></html>\n");
    exit(0);
}


const char *
http_getenv(const char *name)
{
    char            *result;

    result = getenv(name);
    if (!result)
    {
        http_fatal
        (
            http_error_internal_server,
            "Environment variable $%s not set.",
            name
        );
    }
    return result;
}


static const char *
http_getenv(const char *name, const char *dflt)
{
    char *result = getenv(name);
    if (!result)
        return dflt;
    return result;
}


void
html_escape_charstar(const char *s)
{
    html_escape_string(nstring(s));
}


void
html_escape_string(string_ty *s)
{
    html_escape_charstar(s->str_text);
}


void
html_escape_string(const nstring &s)
{
    fputs(s.url_quote().c_str(), stdout);
}


void
html_encode_charstar(const char *s)
{
    html_encode_string(nstring(s));
}


void
html_encode_string(string_ty *s)
{
    html_encode_charstar(s->str_text);
}


void
html_encode_string(const nstring &s)
{
    fputs(s.html_quote(true).c_str(), stdout);
}


nstring
http_sanitize_content_type(const nstring &content_type)
{
    const char *s = content_type.c_str();
    const char *semicolon = s;
    while (*semicolon && *semicolon != ';' && *semicolon != ' ')
        ++semicolon;
    nstring left(s, semicolon - s);
    if (left == "text/html")
        return content_type;
    while (*semicolon == ';' || *semicolon == ' ')
        ++semicolon;
    nstring right;
    if (*semicolon)
    {
        const char *end = s + content_type.size();
        right = "; " + nstring(semicolon, end - semicolon);
    }
    if
    (
        left.starts_with("text/")
    ||
        left == "application/x-awk"
    ||
        left == "application/x-gawk"
    ||
        left == "application/x-nawk"
    ||
        left == "application/x-perl"
    ||
        (left.starts_with("application/x-") && left.ends_with("script"))
    )
    {
        if (right.empty())
            right = "; charset=us-ascii";
        return "text/plain" + right;
    }
    return content_type;
}


void
http_content_type_header(string_ty *filename)
{
    os_become_orig();
    nstring content_type = os_magic_file(filename);
    os_become_undo();
    content_type = http_sanitize_content_type(content_type);
    assert(!content_type.empty());
    printf("Content-Type: %s\n", content_type.c_str());
}


static void
emit_project_attribute(project *pp, change::pointer cp, const char *cname)
{
    if (!pp && cp)
        pp = cp->pp;
    if (pp && (!cp || cp->bogus))
        cp = pp->change_get();
    if (!cp)
        return;
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->project_specific)
        return;
    string_ty *name = str_from_c(cname);
    for (size_t j = 0; j < pconf_data->project_specific->length; ++j)
    {
        attributes_ty *ap = pconf_data->project_specific->list[j];
        if
        (
            ap->name
        &&
            ap->value
        &&
            0 == strcasecmp(name->str_text, ap->name->str_text)
        )
        {
            printf("%s\n", ap->value->str_text);
        }
    }
    str_free(name);
}


void
html_header_ps(project *pp, change::pointer cp)
{
    emit_project_attribute(pp, cp, "html:body-begin");
}


void
html_footer(project *pp, change::pointer cp)
{
    time_t tmp = now();
    printf("<hr>\n");
    printf("This page was generated by <em>%s</em>\n", progname_get());
    printf("version %s\n", version_stamp());
    char buffer[BUFSIZ];
    // gcc 3.3 doesn't like %c
    if (strftime(buffer, BUFSIZ, "%a %d %b %Y %H:%M:%S %Z", localtime(&tmp)))
        printf("on %s.\n", buffer);
    else
        printf("on %.24s.\n", ctime(&tmp));
    emit_project_attribute(pp, cp, "html:body-end");
    printf("</body></html>\n");
}


static void
emit_project_stylesheet(project *pp)
{
    //
    // Netscape 4.x has numerous CSS bugs, two of which need mentioning.
    // 1. If a style sheet is not present Netscape says 404 Not found, when
    // it should silently ignore it.  2. Style sheets who's media is not
    // "screen" will be ignored.  Fortunately we can use (2) to get around (1).
    //
    if (pp && !pp->is_a_trunk())
        emit_project_stylesheet(pp->parent_get());
    else
    {
        printf("<style type=\"text/css\">\n"
            "tr.even-group { background-color: #CCCCCC; }\n"
            "body { background-color: white; }\n"
            "</style>\n");
        printf("<link rel=\"stylesheet\" type=\"text/css\" "
            "href=\"/aedefault.css\" media=\"all\">\n");
    }
    if (pp)
    {
        printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/");
        html_escape_string(project_name_get(pp));
        printf(".css\" media=\"all\">\n");
    }
}


void
html_header(project *pp, change::pointer cp)
{
    if (cp && !pp)
        pp = cp->pp;

    printf("Content-Type: text/html\n\n");
    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\""
        "\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n");
    printf("<html><head>\n");
    printf("<meta name=\"ROBOTS\" content=\"NOINDEX, NOFOLLOW\">\n");
    printf("<meta name=\"GENERATOR\" content=\"aegis-%s\">\n", version_stamp());
    printf("<meta http-equiv=\"Content-Type\" "
        "content=\"text/html; charset=ISO-8859-1\">\n");
    emit_project_stylesheet(pp);
    emit_project_attribute(pp, cp, "html:meta");
}


void
emit_change(change::pointer cp)
{
    long            n;

    emit_project(cp->pp);
    if (cp->bogus)
        return;
    printf(",<br>\nChange <a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(cp->pp));
    n = magic_zero_decode(cp->number);
    printf(".C%ld/?menu\">%ld</a>", n, n);
}


void
emit_change_but1(change::pointer cp)
{
    if (cp->bogus)
        emit_project_but1(cp->pp);
    else
    {
        emit_project(cp->pp);
        printf(",<br>\nChange %ld", magic_zero_decode(cp->number));
    }
}


const char *
http_script_name(void)
{
    //
    // We cache the results, so this fairly expensive operation is only
    // done once.
    //
    static nstring result;
    if (result.empty())
    {
        //
        // Get the script name.  It must exist.
        //
        nstring script_name = http_getenv("SCRIPT_NAME", "/cgi-bin/aeget");

        //
        // Get the server name.  Default it if it doesn't exist.
        //
        nstring server_name = http_getenv("SERVER_NAME", "localhost");

        //
        // Get the server port.  Default it if it doesn't exist.
        // Make sure that it is a number and is in range.
        //
        nstring server_port = http_getenv("SERVER_PORT", "80");
        int port = atoi(server_port.c_str());
        if (port <= 0 || port >= 0x10000)
        {
            http_fatal
            (
                http_error_internal_server,
                "SERVER_PORT of \"%s\" not known",
                server_port.c_str()
            );
        }

        //
        // Get the server protocol.  Default it if it does not exist.
        // We only grok HTTP, so barf if it's anything alse.
        //
        nstring server_protocol = http_getenv("SERVER_PROTOCOL", "HTTP/1.1");
        if (!server_protocol.upcase().starts_with("HTTP"))
        {
            http_fatal
            (
                http_error_internal_server,
                "SERVER_PROTOCOL of \"%s\" not known",
                server_protocol.c_str()
            );
        }

        //
        // Get the request scheme.  Default to http://.
        //
        const char *script_uri = getenv("SCRIPT_URI");
        if (!script_uri)
            script_uri = "";
        nstring request_scheme = "http";
        const char *colon = strchr(script_uri, ':');
        if (colon)
            request_scheme = nstring(script_uri, colon - script_uri);

        //
        // Assemble the script name from the various components to
        // include the protocol, host and port sections as well as the
        // script itself.
        //
        result =
            (
                request_scheme + "://"
            +
                server_name
            +
                (port == 80 ? "" : nstring::format(":%d", port))
            +
                script_name
            );
    }
    return result.c_str();
}


void
emit_project_href(project *pp)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf("/\">");
}


void
emit_project_href(project *pp, const char *modifier, ...)
{
    assert(modifier);
    va_list ap;
    va_start(ap, modifier);
    char buffer[1000];
    vsnprintf(buffer, sizeof(buffer), modifier, ap);
    va_end(ap);

    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf("/");
    if (buffer[0])
        printf("?%s", buffer);
    printf("\">");
}


void
emit_change_href_n(project *pp, long n, const char *modifier)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf(".C%ld/", n);
    if (modifier && *modifier)
        printf("?%s", modifier);
    printf("\">");
}


void
emit_change_href(change::pointer cp, const char *modifier)
{
    if (cp->bogus)
        emit_project_href(cp->pp, "%s", modifier);
    else
        emit_change_href_n(cp->pp, magic_zero_decode(cp->number), modifier);
}


void
emit_change_uuid_href(change::pointer cp, const nstring &uuid,
    const nstring &prefix, const nstring &suffix)
{
    if (cp->bogus)
        emit_project_href(cp->pp, "%s", "menu");
    else
    {
        assert(universal_unique_identifier_valid(cp->uuid_get()));
        printf
        (
            "<a href=\"%s%s%s\">",
            prefix.c_str(),
            uuid_translate(uuid.c_str()).c_str(),
            suffix.c_str()
        );
    }
}

void
emit_file_href(change::pointer cp, const nstring &filename,
    const char *modifier)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(cp->pp));
    if (!cp->bogus)
        printf(".C%ld", magic_zero_decode(cp->number));
    printf("/");
    html_escape_string(filename);
    if (modifier && *modifier)
        printf("?%s", modifier);
    printf("\">");
}


void
emit_file_href(change::pointer cp, string_ty *filename, const char *modifier)
{
    emit_file_href(cp, nstring(filename), modifier);
}


void
emit_rect_image(int width, int height, const char *label, int hspace)
{
    if (width < 3)
        width = 3;
    if (height < 3)
        height = 3;
    printf
    (
        "<img src=\"%s/?rect+width=%d+height=%d",
        http_script_name(),
        width,
        height
    );
    if (label && *label)
    {
        printf("+label=");
        html_escape_charstar(label);
    }
    printf("\" width=%d height=%d", width, height);
    if (hspace >= 0)
    {
#ifndef USE_STYLES
        printf(" hspace=%d", hspace);
#else
        printf(" style=\"left:%d;right:%d;\"", hspace, hspace);
#endif
    }
    printf(">");
}


void
emit_rect_image_rgb(int width, int height, const char *color, int hspace)
{
    if (width < 3)
        width = 3;
    if (height < 3)
        height = 3;
    printf
    (
        "<img\nsrc=\"%s/?rect+width=%d+height=%d",
        http_script_name(),
        width,
        height
    );
    if (color && *color)
        printf("+color=%s", color);
    printf("\" width=%d height=%d", width, height);
    if (hspace >= 0)
    {
#ifndef USE_STYLES
        printf(" hspace=%d", hspace);
#else
        printf(" style=\"left:%d;right:%d;\"", hspace, hspace);
#endif
    }
    printf(">");
}


bool
modifier_test(string_list_ty *modifiers, const char *name)
{
    for (size_t j = 0; j < modifiers->nstrings; ++j)
    {
        if (0 == strcasecmp(modifiers->string[j]->str_text, name))
        {
            return true;
        }
    }
    return false;
}


bool
modifier_test_and_clear(string_list_ty *modifiers, const char *name)
{
    for (size_t j = 0; j < modifiers->nstrings; ++j)
    {
        if (0 == strcasecmp(modifiers->string[j]->str_text, name))
        {
            modifiers->remove(modifiers->string[j]);
            return true;
        }
    }
    return false;
}


void
emit_rss_icon_with_link(project *pp, const nstring &rss_filename)
{
    printf
    (
        "<a href=\"%s/%s/?rss+%s\"><img src=\"%s/icon/rss.gif\" border=0 "
            "alt=\"RSS\"></a>\n",
        http_script_name(),
        project_name_get(pp).c_str(),
        rss_filename.c_str(),
        http_script_name()
    );
}


void
emit_rss_meta_data(project *pp, const nstring &rss_filename)
{
    printf
    (
        "<link rel=\"alternate\" type=\"application/rss+xml\" "
            "href=\"%s/%s/?rss+%s\"/>\n",
        http_script_name(),
        pp->name_get()->str_text,
        rss_filename.c_str()
    );
}


// vim: set ts=8 sw=4 et :
