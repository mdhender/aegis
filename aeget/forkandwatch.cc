//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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
//	along with this program.  If not, see
//	<http://www.gnu.org/licenses/>,
//

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <common/env.h>
#include <common/error.h>
#include <common/quit.h>
#include <common/safe_strsig.h>
#include <common/trace.h>
#include <libaegis/http.h>
#include <libaegis/os.h>

#include <aeget/forkandwatch.h>


static void
html_encode_file(string_ty *fn)
{
    int             fd;

    fd = open(fn->str_text, O_RDONLY);
    if (fd >= 0)
    {
	for (;;)
	{
	    int             n;
	    char            buffer[1025];

	    n = read(fd, buffer, sizeof(buffer) - 1);
	    if (n <= 0)
		break;
	    buffer[n] = 0;
	    html_encode_charstar(buffer);
	}
	close(fd);
    }
}


void
fork_and_watch(void)
{
    int             child;
    int             pfd[2];
    int             n;
    int             fd;
    int             a;
    int             b;
    int             c;
    int             status;
    string_ty       *stdout_filename;
    string_ty       *stderr_filename;

    trace(("fork_and_watch()\n{\n"));
    stdout_filename = os_edit_filename(0);
    stderr_filename = os_edit_filename(0);
    os_become_must_not_be_active();

    //
    // fork to get a process to do the command
    //
    switch (child = fork())
    {
    case -1:
	http_fatal(http_error_internal_server, "fork");
	// NOTREACHED

    default:
	//
	// The parent process waits
	//
	break;

    case 0:
	//
	// We are about to break stdin, stdout and sterr.  These file
	// descriptors are used by programs to determine the terminal
	// size.  Set the appropriate environment variables to that the
	// sub-program agrees with Aegis.
	//
	env_set_page();

	//
	// Redirect stderr.
	//
	fd = creat(stderr_filename->str_text, 0666);
	if (fd < 0)
	    nfatal("open %s", stderr_filename->str_text);
	if (close(2))
	    nfatal("close stderr");
	n = dup(fd);
	if (n < 0)
	    nfatal("%s: %d: dup", __FILE__, __LINE__);
	if (n != 2)
	    fatal_raw("dup gave %d, not 2 (bug)", n);
	close(fd);

	//
	// Redirect stdout.
	//
	fd = creat(stdout_filename->str_text, 0600);
	if (fd < 0)
	    nfatal("open %s", stdout_filename->str_text);
	if (close(1))
	    nfatal("close stdout");
	n = dup(fd);
	if (n < 0)
	    nfatal("%s: %d: dup", __FILE__, __LINE__);
	if (n != 1)
	    fatal_raw("dup gave %d, not 1 (bug)", n);
	close(fd);

	//
	// Redirect stdin from a broken pipe.
	//
	if (pipe(pfd))
	    nfatal("%s: %d: pipe", __FILE__, __LINE__);
	if (close(0))
	    nfatal("%s: %d: close stdin", __FILE__, __LINE__);
	n = dup(pfd[0]);
	if (n < 0)
	    nfatal("%s: %d: dup", __FILE__, __LINE__);
	if (n != 0)
	    fatal_raw("dup gave %d, not 0 (bug)", n);
	if (close(pfd[0]) || close(pfd[1]))
	    nfatal("close pipe ends");

	//
	// The child returns to the caller.
	// (The parent will watch it.)
	//
	return;
    }

    if (os_waitpid(child, &status))
	http_fatal(http_error_internal_server, "wait: %s", strerror(errno));
    a = (status >> 8) & 0xFF;
    b = (status >> 7) & 1;
    c = status & 0x7F;
    switch (c)
    {
    case 0x7F:
	//
	// process was stopped,
	// since we didn't do it, treat it as an error
	//
	http_fatal(http_error_internal_server, "command stopped");
	// NOTREACHED

    case 0:
	// normal termination
	break;

    default:
	//
	// process dies from unhandled condition
	//
	if (!http_fatal_noerror)
	    printf("Status: 500 Internal server error\n");
	html_header(0, 0);
	printf("<title>Error</title></head><body>\n");
	html_header_ps(0, 0);
	printf("<h1>Error</h1>\n");
	printf("The request failed because:\n<em>");
	printf("command terminated by %s", safe_strsignal(c));
	if (b)
	    printf(" (core dumped)");
	printf("</em>\n<p>\n");
	printf("The following error message was produced:\n<blockquote><tt>");
	html_encode_file(stderr_filename);
	printf("</tt></blockquote>\n");
	html_footer(0, 0);
	quit(0);
    }

    if (a != 0)
    {
	//
	// For errors, we wrap up stderr and send it to them.
	//
	if (!http_fatal_noerror)
	    printf("Status: 500 Internal server error\n");
	html_header(0, 0);
	printf("<title>Error</title></head><body>\n");
	html_header_ps(0, 0);
	printf("<h1>Error</h1>\n");
	printf("The request failed with exit status %d.\n", a);
	printf("The following error message was produced:\n<blockquote><tt>");
	html_encode_file(stderr_filename);
	printf("</tt></blockquote>\n");
	html_footer(0, 0);
    }
    else
    {
	FILE            *fp;
	long            len;
	int             nl;

	//
	// If there is no error, we insert a Content-Length header.
	//
	fp = fopen(stdout_filename->str_text, "rb");
	if (!fp)
	{
	    http_fatal
	    (
		http_error_internal_server,
		"open \"%s\": %s",
		stdout_filename->str_text,
		strerror(errno)
	    );
	}
	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);
	rewind(fp);
	nl = 0;
	for (;;)
	{
	    c = fgetc(fp);
	    if (c == EOF)
		break;
	    --len;
	    if (c != '\n')
		nl = 0;
	    else
	    {
		++nl;
		if (nl == 2)
		{
		    if (len > 0)
			printf("Content-Length: %ld\n", (long)len);
		    putchar('\n');
		    break;
		}
	    }
	    putchar(c);
	}
	for (;;)
	{
	    char            buffer[1 << 13];

	    n = fread(buffer, 1, sizeof(buffer), fp);
	    if (n <= 0)
		break;
	    fwrite(buffer, n, 1, stdout);
	}
	fflush(stdout);
	fclose(fp);
    }
    unlink(stdout_filename->str_text);
    unlink(stderr_filename->str_text);
    trace(("quit(0);\n"));
    trace(("}\n"));
    quit(0);
    // NOTREACHED
}
