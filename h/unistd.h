/*
 *	aegis - project change supervisor
 *
 * MANIFEST: minimal substitute for unistd.h
 */

#ifndef UNISTD_H
#define UNISTD_H

#ifndef NULL
#define	NULL 0
#endif

#ifndef SEEK_SET
#define	SEEK_SET 0
#define	SEEK_CUR 1
#define	SEEK_END 2
#endif

#ifndef F_OK
#define	F_OK 0
#define	X_OK 1
#define	W_OK 2
#define	R_OK 4
#endif

void _exit();
int access();
int acct();
unsigned alarm();
int brk();
int chroot();
int chdir();
int chmod();
int chown();
int close();
char *ctermid();
char *cuserid();
int dup();
int dup2();
int execl();
int execle();
int execlp();
int execv();
int execve();
int execvp();
void exit();
int fchdir();
int fchown();
pid_t fork();
long fpathconf();
int fsync();
int ftruncate();
char *getcwd();
gid_t getegid();
uid_t geteuid();
gid_t getgid();
int getgroups();
char *getlogin();
pid_t getpgrp();
pid_t getpid();
pid_t getppid();
pid_t getsid();
uid_t getuid();
int ioctl();
int isatty();
int lchown();
int link();
int lockf();
off_t lseek();
int mincore();
int nice();
long pathconf();
int pause();
int pipe();
void profil();
int ptrace();
int read();
int readlink();
int rmdir();
void *sbrk();
int setgid();
int setegid();
int setgroups();
int setpgid();
int setpgrp();
pid_t setsid();
int setuid();
int seteuid();
int symlink();
void sync();
unsigned sleep();
long sysconf();
pid_t tcgetpgrp();
int tcsetpgrp();
int truncate();
char *ttyname();
int unlink();
int vfork();
int wait();
int waitpid();
int write();

#endif /* UNISTD_H */
