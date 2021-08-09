#include<stdio.h>
#include<pthread.h>

pthread_mutex_t lock;


int
main()
{
	pthread_mutex_lock(&lock);

	pthread_mutex_unlock(&lock);
}
