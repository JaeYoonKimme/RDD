#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;

void  
*worker1()
{
	pthread_mutex_lock(&lock1);
	
	pthread_mutex_lock(&lock2);

	pthread_mutex_lock(&lock3);	

	printf("Passed\n");

	pthread_mutex_unlock(&lock3);

	pthread_mutex_unlock(&lock2);

	pthread_mutex_unlock(&lock1);
}

void 
*worker2()
{
	pthread_mutex_lock(&lock3);

	pthread_mutex_lock(&lock1);

	printf("Passed\n");

	pthread_mutex_unlock(&lock1);

	pthread_mutex_unlock(&lock3);
}

int 
main ()
{	

	pthread_t thread1, thread2;

	pthread_create(&thread1, NULL, worker1, NULL);
	pthread_create(&thread2, NULL, worker2, NULL);

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);

}
