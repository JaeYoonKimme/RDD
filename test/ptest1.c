#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

pthread_mutex_t lock1,lock2,lock3,lock4 = PTHREAD_MUTEX_INITIALIZER;

void
*worker1()
{
	pthread_mutex_lock(&lock1);
	pthread_mutex_unlock(&lock1);
	sleep(1);
	pthread_mutex_lock(&lock2);
	pthread_mutex_unlock(&lock2);
}
void
*worker2()
{
	pthread_mutex_lock(&lock2);
	pthread_mutex_unlock(&lock2);
	sleep(2);
	pthread_mutex_lock(&lock3);
	pthread_mutex_unlock(&lock3);
}
void
*worker3()
{
	pthread_mutex_lock(&lock3);
	pthread_mutex_unlock(&lock3);
	sleep(3);
	pthread_mutex_lock(&lock4);
	pthread_mutex_unlock(&lock4);
}
void
*worker4()
{
	pthread_mutex_lock(&lock4);
	pthread_mutex_unlock(&lock4);
	sleep(4);
	pthread_mutex_lock(&lock1);
	pthread_mutex_unlock(&lock1);
}
int
main ()
{
	pthread_t thread[4];

	pthread_create(&thread[0], NULL, worker1, NULL);
	pthread_create(&thread[1], NULL, worker2, NULL);
	pthread_create(&thread[2], NULL, worker3, NULL);	
	pthread_create(&thread[3], NULL, worker4, NULL);	


	for(int i = 0; i < 4; i++){
		pthread_join(thread[i],NULL);
	}
	printf("Done\n");
}
