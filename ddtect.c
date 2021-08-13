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
#include <limits.h>

static pthread_mutex_t chanel_lock;

char * target_path = 0x0;

struct _node {
	void * mutex; 
	struct _node* next;
	int visited;
};	

typedef struct _node node;

struct _thread {
	long tid;
	int n_mutex;
	node ** mutex_list;
};

typedef struct _thread thread;

node * m_list[10] = { 0x0 };
thread * t_list[10] = { 0x0 } ;

void
get_target_path ()
{
	void * buf[10];
	int size = backtrace (buf, 10);
	char ** strings;
	if( (strings = backtrace_symbols (buf, size)) == NULL){
		perror("BackTrace Error : \n");
		exit(2);
	}

	char * tok = strtok(strings[2], "(");

	target_path = tok;
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

void
print_thread_list()
{       
        printf("\n<print thread list>\n");
        
        for(int i = 0; i< 10; i++){
                if(t_list[i] != 0x0){
                        printf("thread %d : %ld -> mutex list : ",i, t_list[i] -> tid);
                        
                        for(int j = 0; j < t_list[i] -> n_mutex; j++){
                                printf("%p ",t_list[i] -> mutex_list[j] -> mutex);
                        }
                        printf("\n");
                }
        }
}

void
print_mutex_list()
{
        printf("\n<print mutex list>\n");

        for(int i =0; i < 10; i++){
                if(m_list[i] != 0x0){
                        printf("mutex %d : %p\n",i,m_list[i]->mutex);
                }
        }
}

void
print_edges()
{
        printf("\n<print edge status>\n");
        for(int i = 0; i < 10; i++){
                if(m_list[i] != 0x0 && m_list[i] -> next != 0x0){
                        printf("edge %p -> %p \n", m_list[i] -> mutex , m_list[i] -> next -> mutex);
                }
        }
}

void
print(char* type, long tid, void * mutex, long addr)
{
	printf("\n\n---------------------------------------------------------------------------\n");
	printf("Given data set : [%s] %ld -> %p\n",type,tid,mutex);
	print_thread_list();
	print_mutex_list();
	print_edges();
}


thread *
thread_check_and_create(long tid)
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
				tmp -> mutex_list = (node **) malloc(sizeof(node*));
				tmp -> mutex_list[tmp -> n_mutex] = 0x0;
				break;	
			}
		}
	}
	
	return tmp;
}	

node *
mutex_check_and_create(void * mutex)
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
make_edge(thread * cur_thread)
{
	if(cur_thread -> n_mutex > 0  /*&& cur_thread -> mutex_list[cur_thread -> n_mutex - 1] != 0x0*/){
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
		if(cur_thread -> mutex_list[i] == cur_node){
			if(i - 1 >= 0 && cur_thread -> mutex_list[i - 1] != 0x0){
				cur_thread->mutex_list[i - 1] -> next = cur_thread -> mutex_list[i] -> next;
			}
				
			cur_thread -> mutex_list[i] = 0x0;
			cur_node -> next = 0x0;

			node ** new = (node **) malloc (sizeof(node) * (cur_thread -> n_mutex - 1));
			for(int j = 0, index = 0; j< cur_thread -> n_mutex; j++){
				if(cur_thread -> mutex_list[j] != 0x0){
					new[index] = cur_thread -> mutex_list[j];
					index ++;
				}
			}
			free(cur_thread -> mutex_list);
			cur_thread -> mutex_list = new;
			cur_thread -> n_mutex -= 1;
		}
	}	
}

void
find_line_and_print(long addr)
{
	char command[128];
	snprintf(command, 128, "addr2line -e %s %lx",target_path,addr);
	
	FILE * fp;
	if((fp = popen(command,"r"))==NULL){
		perror("Popen error :");
	}
	
	char result[128];
	
	printf("\n********Cyclic Dead Lock Occured********\n");
	while(fgets(result,128, fp) != NULL){
		printf("%s",result);
	}
	printf("\n");

	pclose(fp);	
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
check_deadlock(long addr)
{
	for(int i = 0; i < 10; i++){
		if(m_list[i] != 0x0 && find_cycle(m_list[i]) == 1){
			find_line_and_print(addr);
			exit(1);
			break;
		}
	}
}

void 
update(int type, long tid, void* mutex, long addr)
{
	//LOCK
	if(type == 1){
		node * cur_node = mutex_check_and_create(mutex);
		thread * cur_thread = thread_check_and_create(tid);
		
		cur_thread -> mutex_list[cur_thread->n_mutex] = cur_node;	
		make_edge(cur_thread);	
		cur_thread -> n_mutex += 1;	
		cur_thread -> mutex_list = (node **)realloc(cur_thread -> mutex_list, sizeof(node) * cur_thread -> n_mutex);
		
		//DEBUG
		//print("LOCK",tid,mutex,addr);
	
		check_deadlock(addr);
	}

	//UNLOCK
	if(type == 0){
		release(tid,mutex);
		
		//DEBUG
		//print("UNLOCK",tid,mutex,addr);
	}			
}

int
pthread_mutex_lock (pthread_mutex_t *mutex)
{
	if(target_path == 0x0){
		get_target_path();
	}
	
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

	int type = 1;
	long tid = pthread_self();
	long addr = addr_parse();

	pthread_mutex_lock_origin(&chanel_lock);		
	update(type,tid,mutex,addr);
	pthread_mutex_unlock_origin(&chanel_lock);

	
	//printf("pthread_lock [%ld] : %p\n",pthread_self(),mutex);
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
	
	int type = 0;
	long tid = pthread_self();
	long addr = addr_parse();	

	pthread_mutex_lock_origin(&chanel_lock);
	update(type,tid,mutex,addr);
	pthread_mutex_unlock_origin(&chanel_lock);


 	//printf("pthread_unlock [%ld] : %p\n",pthread_self(),mutex);
	return pthread_mutex_unlock_origin(mutex);
}

