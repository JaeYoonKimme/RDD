#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

int 
main()
{
	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open(".ddtrace", O_WRONLY | O_SYNC);

	while (1){
	}
}
