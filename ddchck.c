#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>

struct _node {
	void * mutex; 
	struct _node* next;
	int visited;
};	

typedef struct _node node;

struct _thread {
	long tid;
	int n_mutex;
	node * mutex_list[100];
};

typedef struct _thread thread;

node * m_list[10] = { 0x0 };
thread * t_list[10] = { 0x0 } ;

thread *
thread_chck(long tid)
{
	thread * tmp = 0x0;
	for(int i = 0; i < 10; i++){
		if(t_list[i] != 0x0 && t_list[i] -> tid == tid){
			tmp = t_list[i];
			break;
		}
	}

	if(tmp == 0x0){
		for(int i = 0; i < 10; i++){
			if(t_list[i] == 0x0){
				tmp = (thread*) malloc(sizeof(thread));
				tmp -> tid = tid;
				tmp -> n_mutex = 0;
				t_list[i] = tmp;
				break;	
			}
		}
	}
	
	return tmp;
}	

node *
mutex_chck(void * mutex)
{
	node * tmp;
	for(int i = 0; i < 10; i++){
		if(m_list[i] != 0x0 && m_list[i] -> mutex == mutex){
			return m_list[i];
		}
	}

	for(int i = 0; i < 10; i++){
		if(m_list[i] == 0x0){
			tmp = (node*) malloc(sizeof(node));
			tmp -> mutex = mutex;
			tmp -> next = 0x0;
			tmp -> visited = 0;
			m_list[i] = tmp;
			return tmp;
		}
	}
}

void
print_thread_list()
{
	printf("\n----------------print thread list--------------\n");

	for(int i = 0; i< 10; i++){
		if(t_list[i] != 0x0){
			printf("thread %d : %ld\n",i, t_list[i] -> tid);
		}
	}
}

void
print_mutex_list()
{
	printf("\n---------------print mutex list----------------\n");

	for(int i =0; i < 10; i++){
		if(m_list[i] != 0x0){
			printf("mutex %d : %p\n",i,m_list[i]->mutex);
		}
	}	
}

void
print_graph()
{
	printf("-----------------print edge status--------------\n");
	for(int i = 0; i < 10; i++){
		if(m_list[i] != 0x0 && m_list[i] -> next != 0x0){
			printf("edge %p -> %p \n", m_list[i] -> mutex , m_list[i] -> next -> mutex);
		}
	}
}

void
make_edge(thread * cur_thread)
{
	if(cur_thread -> n_mutex > 0 && cur_thread -> mutex_list[cur_thread -> n_mutex - 1] != 0x0){
		int start = cur_thread -> n_mutex - 1;
		int end = cur_thread -> n_mutex;

		cur_thread -> mutex_list[start] -> next = cur_thread -> mutex_list[end];
	}	
}

void
release(long tid, void* mutex)
{
	node * cur_node;
	thread * cur_thread;
	for(int i = 0; i< 10; i++){
		if(t_list[i] != 0x0 && t_list[i] -> tid == tid){
			cur_thread = t_list[i];
		}
		
		if(m_list[i] != 0x0 && m_list[i] -> mutex == mutex){
			cur_node = m_list[i];
		}
	}

	for(int i = 0; i < cur_thread -> n_mutex ; i++){
		if(cur_thread -> mutex_list[i] != 0x0 && cur_thread -> mutex_list[i] == cur_node){
			if(i - 1 >= 0 && cur_thread -> mutex_list[i - 1] != 0x0){
				cur_thread->mutex_list[i - 1] -> next = cur_thread -> mutex_list[i] -> next;
			}
				
			cur_thread -> mutex_list[i] = 0x0;
			cur_node -> next = 0x0;
		}
	}	
}

int
find_cycle(node * cur)
{
	int found = 0;
	int count = 0;

	for(node * tmp = cur; tmp != 0x0; tmp = tmp -> next){
		if(tmp -> visited == 1){
			found = 1;
			count += 1;
			break;
		}

		else{
			tmp -> visited = 1;
		}
	}

	for(node * tmp = cur; tmp != 0x0; tmp = tmp -> next){
		if(tmp -> visited == 1){
			tmp -> visited = 0;
			count -= 1;
			
			if(count == 0){
				break;
			}
		}
	}
	return found;
}

void
check_deadlock()
{
	for(int i = 0; i < 10; i++){
		if(m_list[i] != 0x0 && find_cycle(m_list[i]) == 1){
			printf("\n********Dead Lock Occured********\n");
			//exit(1);
			break;
		}
	}	
}

void 
update(int type, long tid, void* mutex)
{
	//LOCK
	if(type == 1){
		node * cur_node = mutex_chck(mutex);
		thread * cur_thread = thread_chck(tid);
		
		cur_thread -> mutex_list[cur_thread->n_mutex] = cur_node;	
		make_edge(cur_thread);	
		cur_thread -> n_mutex += 1;	


		printf("---------------------------------------------------------------------------\n");
		printf("Given data set : [LOCK] %ld -> %p\n",tid,mutex);
		print_thread_list();		
		print_mutex_list();						
		print_graph();
		check_deadlock();
		printf("---------------------------------------------------------------------------\n\n\n");
	}

	//UNLOCK
	if(type == 0){
		release(tid,mutex);


		printf("---------------------------------------------------------------------------\n");
		printf("Given data set : [UNLOCK] %ld -> %p\n",tid,mutex);
		print_thread_list();
		print_mutex_list();
		print_graph();
		printf("---------------------------------------------------------------------------\n\n\n");
	}			
}

void
read_s (size_t len, char * data, int fd)
{
	size_t s;
	while(len > 0 && (s= read(fd, data, len)) > 0){
		data +=s;
		len -=s;
	}
}

int 
main()
{
	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open(".ddtrace", O_RDWR | O_SYNC);

	while (1){
		int type = -1;
		void * addr = 0x0;
		long tid = -1;
		
		read_s(sizeof(type), (char*)&type, fd);
		read_s(sizeof(tid), (char*)&tid, fd);	
		read_s(sizeof(addr), (char*)&addr, fd);
		
		update(type,tid,addr);
			
		//if(type != -1 && addr != 0x0 && tid != -1){
			//printf("%d %ld %p\n",type,tid,addr);
		//}
	}
}
