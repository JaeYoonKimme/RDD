#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	/*
	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
			perror("fail to open fifo : ");
			exit(1);
		}
	}
	*/

	int (*pthread_mutex_lock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;
	
	pthread_mutex_lock_origin = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if((error == dlerror()) != 0x0){
		exit(1);
	}

	pthread_self = dlsym(RTLD_NEXT, "pthread_self");
	if((error == dlerror()) != 0x0){
		exit(1);
	}

	printf("pthread_lock [%ld] : %p\n",pthread_self(),mutex);
	//char buf[50];
	//snprintf(buf, 50, "pthread_lock : %p\n",mutex);
	
	//int fd = open("testfile", O_WRONLY | O_SYNC);	

	return pthread_mutex_lock_origin(mutex);
}

int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
	int (*pthread_mutex_unlock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;

	pthread_mutex_unlock_origin = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error == dlerror()) != 0x0){
		exit(1);
	}

	pthread_self = dlsym(RTLD_NEXT, "pthread_self");
	if((error == dlerror()) != 0x0){
		exit(1);	
	}
	
	printf("pthread_unlock [%ld] : %p\n",pthread_self(),mutex);
	//char buf[50];
	//snprintf(buf, 50, "pthread_unlock : %p\n",mutex);

	return pthread_mutex_unlock_origin(mutex);
}

