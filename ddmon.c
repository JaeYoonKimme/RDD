#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>

void
write_s (size_t len, char * data, int fd)
{
	size_t s;
	while(len > 0 && (s = write(fd, data, len)) > 0){
		data += s;
		len -= s;
	}
}

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{	
	int (*pthread_mutex_lock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;
	
	pthread_mutex_lock_origin = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if((error = dlerror()) != 0x0){
		perror("dlsym error (lock)");
		exit(1);
	}

	pthread_self = dlsym(RTLD_NEXT, "pthread_self");
	if((error = dlerror()) != 0x0){
		perror("dlsym error (self)");
		exit(1);
	}

	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
 			perror("fail to open fifo : ");
 			exit(1);
 		}
	}
	int fd = open(".ddtrace",O_WRONLY | O_SYNC);

	printf("pthread_lock [%ld] : %p\n",pthread_self(),mutex);

	flock(fd, LOCK_EX) ;

	int type = 1;
	long tid = pthread_self();

	//backtrace test
	void * buf[10];
	int size = backtrace (buf, 10);

	char ** strings;
	strings = backtrace_symbols (buf, size);
	printf("------------backtrace info----------\n");
	for(int i = 0; i< size; i++){
		printf("%s\n",strings[i]);
	}
	printf("------------------------------------\n");
	//

	write_s(sizeof(type),(char*)&type, fd);
	write_s(sizeof(tid), (char*)tid, fd);
	write_s(sizeof(mutex), (char*)&mutex, fd);

	flock(fd, LOCK_UN) ;

	close(fd);
	return pthread_mutex_lock_origin(mutex);
}



int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
	int (*pthread_mutex_unlock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;

	pthread_mutex_unlock_origin = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0){
		exit(1);
	}

	pthread_self = dlsym(RTLD_NEXT, "pthread_self");
	if((error = dlerror()) != 0x0){
		exit(1);	
	}
	
	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
 				perror("fail to open fifo : ");
 				exit(1);
 		}
 	}

 	printf("pthread_unlock [%ld] : %p\n",pthread_self(),mutex);

 	int fd = open(".ddtrace",O_WRONLY | O_SYNC);

 	flock(fd, LOCK_EX) ;
 	int type = 0;
 	write_s(sizeof(type),(char*)&type, fd);
 	
 	long tid = pthread_self();
 	write_s(sizeof(tid), (char*)tid, fd);
 	
	write_s(sizeof(mutex), (char*)&mutex, fd);

	flock(fd, LOCK_UN) ;

 	close(fd);

	return pthread_mutex_unlock_origin(mutex);
}

