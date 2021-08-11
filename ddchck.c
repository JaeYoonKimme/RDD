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
	void * addr;
	long tid;
	struct _node* edge[9]; 
	int visited;
};

typedef struct _node node;
typedef struct _node * node_ptr;

node_ptr list[10] = {0x0};

void *
init_node(long tid, void * addr)
{
	node_ptr tmp = (node_ptr) malloc(sizeof(node));
	tmp -> addr = addr;
	tmp -> tid = tid;
	int visited = 0;
	for(int i = 0; i<9; i++){
		tmp -> edge[i] = 0x0;
	}

	return tmp;
}

void
print_list()
{
	for(int i = 0; i < 10; i++){
		if(list[i] != 0x0){
			printf("list[%d]- tid : %ld, mutex_addr : %p\n",i,list[i]->tid,list[i]->addr);
			
			for(int j=0; j<9; j++){
				if(list[i] -> edge[j] != 0x0){
					node_ptr tmp = list[i] -> edge[j];
					printf("edge %p %ld -> %p %ld\n", list[i]->addr, list[i]->tid, tmp->addr, tmp->tid);
					
				}
			}
		}	
	}
	printf("\n");
}

int
check_list(void * addr)
{
	for(int i = 0; i<10; i++){
		if(list[i] != 0x0){
			if(list[i] -> addr == addr){
				return 1;
			}
		}
	}
	return 0;
}

void
make_edge(long tid, void* addr)
{
	node_ptr end ;
	for(int i = 0; i< 10; i++){
		if(list[i] != 0x0 && list[i] -> addr == addr){
			end = list[i];
		}
	}

	for(int i = 0; i< 10; i++){
		if(list[i] != 0x0 && list[i] -> tid == tid && list[i] -> addr != addr){
			node_ptr start = list[i];
			for(int j=0; j<9; j++){
				if(start -> edge[j] == 0x0){
					start -> edge[j] = end;
					break;
				}
			}
		}
	}
}

void
update_list(int type,long tid, void * addr)
{
	if(type == 1){
		if(!check_list(addr)){
			node_ptr new = init_node(tid,addr);		
			for(int i = 0; i < 10; i++){
				if(list[i] == 0x0){
					list[i] = new;
					break;
				}
			}
		}
		make_edge(tid, addr);	
		print_list();
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
		
		//flock(fd, LOCK_EX) ;
		read_s(sizeof(type), (char*)&type, fd);
		read_s(sizeof(tid), (char*)&tid, fd);	
		read_s(sizeof(addr), (char*)&addr, fd);
		//flock(fd, LOCK_UN) ;
		
		update_list(type,tid,addr);
		
		if(type != -1 && addr != 0x0 && tid != -1){
			//printf("%d %ld %p\n",type,tid,addr);
		}
	}
}
