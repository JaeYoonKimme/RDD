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
#include <string.h>
#include <pthread.h>

static pthread_mutex_t chanel_lock;

void
write_s (size_t len, char * data, int fd)
{
	size_t s;
	while(len > 0 && (s = write(fd, data, len)) > 0){
		data += s;
		len -= s;
	}
}

long
addr_parse ()
{
	void * buf[10];
	int size = backtrace (buf, 10);
	char ** strings;
	if( (strings = backtrace_symbols (buf, size)) == NULL){
		perror("BackTrace Error : \n");
		exit(2);
	}

	char * tok = strtok(strings[2], "+");
	tok = strtok(NULL, ")");

	return strtol(tok,NULL,16) - 5;
}

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{	
	int (*pthread_mutex_lock_origin)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;
	
	pthread_mutex_lock_origin = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if((error = dlerror()) != 0x0){
		perror("dlsym error (lock)");
		exit(1);
	}
	pthread_mutex_unlock_origin = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	if((error = dlerror()) != 0x0){
		perror("dlsym error (unlock)");
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

	int type = 1;
	long tid = pthread_self();
	long addr = addr_parse();

	pthread_mutex_lock_origin(&chanel_lock);		
	write_s(sizeof(type),(char*)&type, fd);
	write_s(sizeof(tid), (char*)tid, fd);
	write_s(sizeof(mutex), (char*)&mutex, fd);
	write_s(sizeof(addr), (char*)&addr, fd);
	pthread_mutex_unlock_origin(&chanel_lock);

	close(fd);
	
	printf("pthread_lock [%ld] : %p\n",pthread_self(),mutex);
	return pthread_mutex_lock_origin(mutex);	
}



int
pthread_mutex_unlock (pthread_mutex_t *mutex)
{
	int (*pthread_mutex_lock_origin)(pthread_mutex_t *mutex);
	int (*pthread_mutex_unlock_origin)(pthread_mutex_t *mutex);
	pthread_t (*pthread_self)(void);
	char * error;
	pthread_mutex_lock_origin = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	if((error = dlerror()) != 0x0){
		exit(1);
	}
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
 	int fd = open(".ddtrace",O_WRONLY | O_SYNC);



	int type = 0;
	long tid = pthread_self();
	long addr = addr_parse();	

	pthread_mutex_lock_origin(&chanel_lock);
 	write_s(sizeof(type),(char*)&type, fd);
 	write_s(sizeof(tid), (char*)tid, fd);
	write_s(sizeof(mutex), (char*)&mutex, fd);
	write_s(sizeof(addr), (char*)&addr, fd);
	pthread_mutex_unlock_origin(&chanel_lock);

 	close(fd);

 	printf("pthread_unlock [%ld] : %p\n",pthread_self(),mutex);
	return pthread_mutex_unlock_origin(mutex);
}

