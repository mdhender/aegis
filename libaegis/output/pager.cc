//
//	aegis - project change supervisor
//	Copyright (C) 1992-1995, 1997, 1999, 2002-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to pipe output through paginator
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <sys/stat.h> // for umask prototype

#include <arglex2.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <option.h>
#include <os.h>
#include <output.h>
#include <output/pager.h>
#include <output/private.h>
#include <output/stdout.h>
#include <str.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


typedef struct output_pager_ty output_pager_ty;
struct output_pager_ty
{
    output_ty	    inherited;
    FILE	    *deeper;
    int		    pid;
    string_ty	    *pager;
    int		    bol;
};

static int	option_pager_flag = -1;
static output_ty *singleton;


void
option_pager_set(int n, void (*usage)(void))
{
    if (option_pager_flag == 0 && n == 0)
	duplicate_option_by_name(arglex_token_pager_not, usage);
    if (option_pager_flag >= 1 && n != 0)
	duplicate_option_by_name(arglex_token_pager, usage);
    if (option_pager_flag >= 0)
    {
	mutually_exclusive_options
	(
	    arglex_token_pager,
	    arglex_token_pager_not,
	    usage
	);
    }
    option_pager_flag = (n != 0);
}


static int
option_pager_get(void)
{
    user_ty	    *up;

    if (option_pager_flag < 0)
    {
	up = user_executing(0);
	option_pager_flag = user_pager_preference(up);
	user_free(up);
    }
    return option_pager_flag;
}


static void
pipe_open(output_pager_ty *this_thing)
{
    sub_context_ty  *scp;
    int		    uid;
    int		    gid;
    int		    um;
    FILE	    *fp;
    int		    fd[2];
    const char      *cmd[4];
    int		    pid;
    int             errno_old;

    env_set_page();
    fp = 0;
    os_become_orig_query(&uid, &gid, &um);
    if (pipe(fd))
	nfatal("pipe()");
    switch (pid = fork())
    {
    case 0:
	undo_cancel();
	while (os_become_active())
	    os_become_undo();
	cmd[0] = "sh";
	cmd[1] = "-c";
	cmd[2] = this_thing->pager->str_text;
	cmd[3] = 0;
	close(fd[1]);
	close(0);
	if (dup(fd[0]) != 0)
	    fatal_raw("dup was wrong");
	close(fd[0]);
	os_setgid(gid);
	os_setuid(uid);
	umask(um);
	execvp(cmd[0], (char **)cmd);

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", this_thing->pager);
	fatal_intl(scp, i18n("exec \"$filename\": $errno"));
	// NOTREACHED

    case -1:
	nfatal("fork()");
	break;

    default:
	this_thing->pid = pid;
	close(fd[0]);
	fp = fdopen(fd[1], "w");
	if (!fp)
	    nfatal("fdopen");
	this_thing->deeper = fp;
	break;
    }
}


static void
cleanup(int n)
{
    trace(("output_pager::cleanup(n = %d)\n{\n", n));
    if (singleton)
    {
	output_ty	*p;

	p = singleton;
	singleton = 0;
	output_delete(p);
    }
    trace(("}\n"));
}


static void
pager_error(output_pager_ty *this_thing)
{
    sub_context_ty  *scp;
    int             errno_old;

    errno_old = errno;
    scp = sub_context_new();
    sub_errno_setx(scp, errno_old);
    sub_var_set_string(scp, "File_Name", this_thing->pager);
    fatal_intl(scp, i18n("write $filename: $errno"));
    // NOTREACHED
}


static void
output_pager_destructor(output_ty *fp)
{
    output_pager_ty *this_thing;
    int		    status;

    trace(("output_pager::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (output_pager_ty *)fp;
    assert(this_thing->deeper);

    //
    // nuke the singelton
    //
    assert(singleton == fp);
    singleton = 0;

    //
    // write the last of the output
    //
    if (fflush(this_thing->deeper))
	pager_error(this_thing);

    //
    // close the paginator
    //
    fclose(this_thing->deeper);
    this_thing->deeper = 0;
    os_waitpid(this_thing->pid, &status);
    this_thing->pid = 0;
    trace(("}\n"));
}


static string_ty *
output_pager_filename(output_ty *fp)
{
    output_pager_ty *this_thing;

    this_thing = (output_pager_ty *)fp;
    return this_thing->pager;
}


static long
output_pager_ftell(output_ty *fp)
{
    return -1;
}


static void
output_pager_write(output_ty *fp, const void *data, size_t len)
{
    output_pager_ty *this_thing;

    this_thing = (output_pager_ty *)fp;
    if (fwrite(data, 1, len, this_thing->deeper) == 0)
	pager_error(this_thing);
    if (len > 0)
	this_thing->bol = (((const char *)data)[len - 1] == '\n');
}


static void
output_pager_flush(output_ty *fp)
{
    output_pager_ty *this_thing;

    this_thing = (output_pager_ty *)fp;
    if (fflush(this_thing->deeper))
	pager_error(this_thing);
}


static void
output_pager_eoln(output_ty *fp)
{
    output_pager_ty *this_thing;

    this_thing = (output_pager_ty *)fp;
    if (!this_thing->bol)
	output_fputc(fp, '\n');
}


static output_vtbl_ty vtbl =
{
    sizeof(output_pager_ty),
    output_pager_destructor,
    output_pager_filename,
    output_pager_ftell,
    output_pager_write,
    output_pager_flush,
    output_generic_page_width,
    output_generic_page_length,
    output_pager_eoln,
    "pager",
};


output_ty *
output_pager_open(void)
{
    output_ty	    *result;
    output_pager_ty *this_thing;

    trace(("output_pager::open()\n{\n"));
    assert(!singleton);

    //
    // If talking to a terminal,
    // send the output through a paginator.
    // If we're not, just use stdout.
    //
    if
    (
	!option_pager_get()
    ||
	option_unformatted_get()
    ||
	os_background()
    ||
	!isatty(0)
    ||
	!isatty(1)
    ||
	singleton
    )
	return output_stdout();

    //
    // register the cleanup function in case of fatal errors
    //
    quit_register(cleanup);

    //
    // open the paginator
    //
    result = output_new(&vtbl);
    this_thing = (output_pager_ty *)result;
    this_thing->deeper = 0;
    this_thing->pid = 0;
    this_thing->pager = user_pager_command((user_ty *)0);
    this_thing->bol = 1;
    os_become_orig();
    pipe_open(this_thing);
    os_become_undo();
    assert(this_thing->deeper);

    //
    // We keep track of open paginators,
    // so we can clean up after them if necessary.
    //
    singleton = result;

    trace(("}\n"));
    return result;
}
