/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992-1995, 1997, 1999, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to pipe output through paginator
 */

#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <sys/stat.h> /* for umask prototype */

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
option_pager_set(n, usage)
    int		    n;
    void	    (*usage)_((void));
{
    if (option_pager_flag == 0 && n == 0)
	duplicate_option_by_name(arglex_token_no_pager, usage);
    if (option_pager_flag >= 1 && n != 0)
	duplicate_option_by_name(arglex_token_pager, usage);
    if (option_pager_flag >= 0)
    {
	mutually_exclusive_options
	(
	    arglex_token_pager,
	    arglex_token_no_pager,
	    usage
	);
    }
    option_pager_flag = (n != 0);
}


static int option_pager_get _((void));

static int
option_pager_get()
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


static void env_set_page _((void));

static void
env_set_page()
{
    char	    buffer[20];

    sprintf(buffer, "%d", option_page_length_get(-1));
    env_set("LINES", buffer);
    sprintf(buffer, "%d", option_page_width_get(-1));
    env_set("COLS", buffer);
}


static void pipe_open _((output_pager_ty *));

static void
pipe_open(this)
    output_pager_ty *this;
{
    sub_context_ty  *scp;
    int		    uid;
    int		    gid;
    int		    um;
    FILE	    *fp;
    int		    fd[2];
    char	    *cmd[4];
    int		    pid;

    fp = 0;
    os_become_orig_query(&uid, &gid, &um);
    if (pipe(fd))
	nfatal("pipe()");
    switch (pid = fork())
    {
    case 0:
	env_set_page();
	undo_cancel();
	while (os_become_active())
	    os_become_undo();
	cmd[0] = "sh";
	cmd[1] = "-c";
	cmd[2] = this->pager->str_text;
	cmd[3] = 0;
	close(fd[1]);
	close(0);
	if (dup(fd[0]) != 0)
	    fatal_raw("dup was wrong");
	close(fd[0]);
	os_setgid(gid);
	os_setuid(uid);
	umask(um);
	execvp(cmd[0], cmd);

	scp = sub_context_new();
	sub_errno_set(scp);
	sub_var_set_string(scp, "File_Name", this->pager);
	fatal_intl(scp, i18n("exec \"$filename\": $errno"));
	/* NOTREACHED */

    case -1:
	nfatal("fork()");
	break;

    default:
	this->pid = pid;
	close(fd[0]);
	fp = fdopen(fd[1], "w");
	if (!fp)
	    nfatal("fdopen");
	this->deeper = fp;
	break;
    }
}


static void cleanup _((int));

static void
cleanup(n)
    int		    n;
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


static void pager_error _((output_pager_ty *));

static void
pager_error(this)
    output_pager_ty *this;
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sub_errno_set(scp);
    sub_var_set_string(scp, "File_Name", this->pager);
    fatal_intl(scp, i18n("write $filename: $errno"));
    /* NOTREACHED */
}


static void output_pager_destructor _((output_ty *));

static void
output_pager_destructor(fp)
    output_ty	    *fp;
{
    output_pager_ty *this;
    int		    status;

    trace(("output_pager::destructor(fp = %08lX)\n{\n", (long)fp));
    this = (output_pager_ty *)fp;
    assert(this->deeper);

    /*
     * nuke the singelton
     */
    assert(singleton == fp);
    singleton = 0;

    /*
     * write the last of the output
     */
    if (fflush(this->deeper))
	pager_error(this);

    /*
     * close the paginator
     */
    fclose(this->deeper);
    this->deeper = 0;
    os_waitpid(this->pid, &status);
    this->pid = 0;
    trace(("}\n"));
}


static string_ty *output_pager_filename _((output_ty *));

static string_ty *
output_pager_filename(fp)
    output_ty	    *fp;
{
    output_pager_ty *this;

    this = (output_pager_ty *)fp;
    return this->pager;
}


static long output_pager_ftell _((output_ty *));

static long
output_pager_ftell(fp)
    output_ty	    *fp;
{
    return -1;
}


static void output_pager_write _((output_ty *, const void *s, size_t));

static void
output_pager_write(fp, data, len)
    output_ty	    *fp;
    const void	    *data;
    size_t	    len;
{
    output_pager_ty *this;

    this = (output_pager_ty *)fp;
    if (fwrite(data, 1, len, this->deeper) == EOF)
	pager_error(this);
    if (len > 0)
	this->bol = (((const char *)data)[len - 1] == '\n');
}


static void output_pager_flush _((output_ty *));

static void
output_pager_flush(fp)
    output_ty	    *fp;
{
    output_pager_ty *this;

    this = (output_pager_ty *)fp;
    if (fflush(this->deeper))
	pager_error(this);
}


static void output_pager_eoln _((output_ty *));

static void
output_pager_eoln(fp)
    output_ty	    *fp;
{
    output_pager_ty *this;

    this = (output_pager_ty *)fp;
    if (!this->bol)
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
output_pager_open()
{
    output_ty	    *result;
    output_pager_ty *this;

    trace(("output_pager::open()\n{\n"));
    assert(!singleton);

    /*
     * If talking to a terminal,
     * send the output through a paginator.
     * If we're not, just use stdout.
     */
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

    /*
     * register the cleanup function in case of fatal errors
     */
    quit_register(cleanup);

    /*
     * open the paginator
     */
    result = output_new(&vtbl);
    this = (output_pager_ty *)result;
    this->deeper = 0;
    this->pid = 0;
    this->pager = user_pager_command((user_ty *)0);
    this->bol = 1;
    os_become_orig();
    pipe_open(this);
    os_become_undo();
    assert(this->deeper);

    /*
     * We keep track of open paginators,
     * so we can clean up after them if necessary.
     */
    singleton = result;

    trace(("}\n"));
    return result;
}
