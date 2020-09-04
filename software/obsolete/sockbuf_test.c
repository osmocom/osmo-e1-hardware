
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>

static int set_int_opt(int fd, int opt, int val)
{
	return setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val));
}

static int get_int_opt(int fd, int opt)
{
	int ret, rc;
	socklen_t optlen = sizeof(ret);

	rc = getsockopt(fd, SOL_SOCKET, opt, &ret, &optlen);
	if (rc < 0)
		return rc;
	return ret;
}

static int set_nonblock(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;

	return fcntl(fd, F_SETFL, flags);
}

static int run_test_socketpair(int tx_buf)
{
	int rc, sd[2];
	uint8_t buf[1024*1024];

	memset(buf, 0, sizeof(buf));

	rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sd);
	if (rc < 0)
		return rc;

	/* set the lowest possible transmit socket buffer */
	set_int_opt(sd[0], SO_SNDBUF, tx_buf);
	set_nonblock(sd[0]);

	rc = write(sd[0], buf, sizeof(buf));
	printf("socketpair: tx_buf %7d: written %7d of %ld\n", tx_buf, rc, sizeof(buf));

	close(sd[0]);
	close(sd[1]);
	return 0;
}

static int run_test_pipe(int tx_buf)
{
	int rc, sd[2];
	uint8_t buf[1024*1024];

	memset(buf, 0, sizeof(buf));

	rc = pipe(sd);
	if (rc < 0)
		return rc;

	set_nonblock(sd[1]);

	rc = write(sd[1], buf, sizeof(buf));
	printf("pipe: tx_buf %7d: written %7d of %ld\n", tx_buf, rc, sizeof(buf));

	close(sd[0]);
	close(sd[1]);
	return 0;
}




int main(int argc, char **argv)
{
	int i;

	for (i = 0; i < 20; i++) {
		run_test_socketpair(1<<i);
	}
	run_test_pipe(0);
}
