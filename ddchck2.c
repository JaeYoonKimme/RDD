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
	struct node* next;
	int visited;
};	

typedef struct _node node;
typedef struct _node * node_ptr;

struct _thread {
	long tid;
	node mutex_list[100];
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
		}
	}

	if(tmp == 0x0){
		for(int i = 0; i < 10; i++){
			if(t_list[i] == 0x0){
				tmp = (thread*) malloc(sizeof(thread));
				tmp -> tid = tid;
				t_list[i] = tmp;	
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
			return tmp;
		}
	}
}

void
make_edge(node * cur_node, thread * cur_thread)
{
	
}

void 
update(int type, long tid, void* mutex)
{
	if(type == 1){
		node * cur_node = mutex_chck(mutex);
		thread * cur_thread = thread_chck(tid);
		make_edge(cur_node,cur_thread);	
		
		
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
		
		if(type != -1 && addr != 0x0 && tid != -1){
			//printf("%d %ld %p\n",type,tid,addr);
		}
	}
}
