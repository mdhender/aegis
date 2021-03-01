/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994, 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/glue.c
 */

#ifndef AEGIS_GLUE_H
#define AEGIS_GLUE_H

#include <ac/stdio.h>

#include <main.h>
#include <config.h>

struct stat;
struct utimbuf;
struct flock;

int copyfile(char *src, char *dst);
int catfile(char *path);
int read_whole_dir(char *path, char **data, long *datalen);
int file_compare(char *, char *);
int file_fingerprint(char *path, char *buf, int max);
int rmdir_bg(char *path);
int rmdir_tree(char *path);

int glue_access(char *path, int mode);
int glue_catfile(char *path);
int glue_chmod(char *path, int mode);
int glue_chown(char *path, int uid, int gid);
int glue_close(int fd);
int glue_creat(char *path, int mode);
int glue_copyfile(char *src, char *dst);
int glue_fclose(FILE *);
int glue_file_compare(char *, char *);
int glue_file_fingerprint(char *path, char *buf, int max);
int glue_fcntl(int fd, int cmd, struct flock *);
int glue_ferror(FILE *);
int glue_fflush(FILE *);
int glue_fgetc(FILE *);
FILE *glue_fopen(char *path, char *mode);
int glue_fputc(int, FILE *);
char *glue_getcwd(char *buf, int max);
int glue_link(char *p1, char *p2);
int glue_lstat(char *path, struct stat *st);
int glue_mkdir(char *path, int mode);
int glue_open(char *path, int mode, int perm);
long glue_pathconf(char *path, int mode);
long glue_read(int fd, void *data, size_t len);
int glue_readlink(char *path, char *buf, int max);
int glue_read_whole_dir(char *path, char **data, long *datalen);
int glue_rename(char *p1, char *p2);
int glue_rmdir(char *path);
int glue_rmdir_bg(char *path);
int glue_rmdir_tree(char *path);
int glue_stat(char *path, struct stat *st);
int glue_symlink(char *name1, char *name2);
int glue_ungetc(int, FILE *);
int glue_unlink(char *path);
int glue_utime(char *path, struct utimbuf *);
int glue_write(int fd, const void *data, long len);
int glue_fwrite(char *, long, long, FILE *);


#ifndef CONF_NO_seteuid
#ifndef aegis_glue_disable

/*
 * when seteuid works,
 * use the functions directly,
 * not the glue functions
 */
#define	glue_access	access
#define	glue_catfile	catfile
#define	glue_chmod	chmod
#define	glue_chown	chown
#define	glue_close	close
#define	glue_creat	creat
#define	glue_copyfile	copyfile
#define	glue_fclose	fclose
#define	glue_fcntl	fcntl
#define	glue_ferror	ferror
#define	glue_fflush	fflush
#define	glue_fgetc	getc
#define glue_file_compare file_compare
#define glue_file_fingerprint file_fingerprint
#define	glue_fopen	fopen
#define	glue_fputc	putc
#define	glue_getcwd	getcwd
#define	glue_link	link
#define	glue_lstat	lstat
#define	glue_mkdir	mkdir
#define	glue_open	open
#define	glue_pathconf	pathconf
#define	glue_read	read
#define	glue_readlink	readlink
#define	glue_read_whole_dir	read_whole_dir
#define	glue_rename	rename
#define	glue_rmdir	rmdir
#define	glue_rmdir_bg	rmdir_bg
#define	glue_rmdir_tree	rmdir_tree
#define	glue_stat	stat
#define	glue_symlink	symlink
#define	glue_ungetc	ungetc
#define	glue_unlink	unlink
#define	glue_utime	utime
#define	glue_write	write
#define	glue_fwrite	fwrite

#endif /* aegis_glue_disable */
#endif /* CONF_NO_seteuid */

#endif /* AEGIS_GLUE_H */
