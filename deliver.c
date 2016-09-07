/*
 * Written by Pedro Martelletto in November 2010.
 * Updated to work with Postfix in April 2012.
 * Public domain.
 */

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#define DEATH 86400  /* 24 hours */
#define MAXTRIES 3  /* 6 seconds max */
#define CHUNKSIZ 8192  /* 8k is reasonable */

char path[MAXPATHLEN];
char host[HOST_NAME_MAX+1];
char data[CHUNKSIZ];

void
timeout(int sig)
{
	unlink(path);
	exit(100);
}

void
try_unlink(const char *path)
{
	if (unlink(path) == -1)
		warn("unlink");
}

int
main(int argc, char **argv)
{
	if (argc != 2)
		errx(EX_USAGE, "wrong syntax");

	const char *maildir = argv[1];
	if (chdir(maildir) != 0)
		err(EX_DATAERR, "chdir");

	if (gethostname(host, sizeof(host)-1) != 0)
		err(EX_OSERR, "gethostname");

	int i;
	for (i = 0; i < MAXTRIES; i++) {
		time_t t = time(NULL);
		if (t == (time_t)-1)
			err(EX_OSERR, "time");

		pid_t p = getpid(); /* no errors are defined for getpid() */

		int r = snprintf(path, sizeof(path), "tmp/%lld.%d.%s",
		    (long long)t, (int)p, host);
		if (r < 0 || (size_t)r >= sizeof(path))
			errx(EX_CANTCREAT, "path too long");

		struct stat sb;
		if (stat(path, &sb) != 0 && errno == ENOENT)
			break;

		sleep(2);
	}

	if (i == MAXTRIES)
		errx(EX_TEMPFAIL, "could not get unique path");

	if (signal(SIGALRM, timeout) == SIG_ERR)
		err(EX_OSERR, "signal");

	/*
	 * IEEE Std 1003.1-2008 does not specify errors for alarm(). At least
	 * {Open,Net,Free}BSD, however, return (unsigned int)-1 on error and
	 * set 'errno' accordingly.
	 */
	if (alarm(DEATH) == (unsigned int)-1)
		err(EX_OSERR, "alarm");

	int fd = open(path, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR);
	if (fd < 0)
		err(EX_CANTCREAT, "open");

	ssize_t r;
	while ((r = read(STDIN_FILENO, data, sizeof(data))) != -1 && r != 0) {
		ssize_t w = write(fd, data, r);
		if (w == -1) {
			try_unlink(path);
			err(EX_IOERR, "write");
		}
		if (w < r) {
			try_unlink(path);
			errx(EX_IOERR, "short write");
		}
	}

	if (r == -1) {
		try_unlink(path);
		err(EX_IOERR, "read");
	}

	if (fsync(fd) != 0) {
		try_unlink(path);
		err(EX_IOERR, "fsync");
	}

	if (close(fd) != 0) {
		try_unlink(path);
		err(EX_IOERR, "close");
	}

	char *newpath = strdup(path);
	if (newpath == NULL) {
		try_unlink(path);
		err(EX_OSERR, "malloc");
	}

	newpath[0] = 'n';
	newpath[1] = 'e';
	newpath[2] = 'w';

	if (link(path, newpath) != 0) {
		try_unlink(path);
		err(EX_CANTCREAT, "link");
	}

	if (unlink(path) == -1)
		err(EX_NOINPUT, "unlink");

	exit(EX_OK);
}
