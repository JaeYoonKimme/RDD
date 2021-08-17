#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>


char * target_path;

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

struct _edge {
	long tid;
	void ** mutex_history;
	int n_history;
	node * start;
	node * end;
	//long addr;
};

typedef struct _edge edge;

node * m_list[10] = { 0x0 };
thread * t_list[10] = { 0x0 } ;

edge ** e_list;
int n_edge = 0;

char ** line_info;
int cnt_line = 0;

int
read_s (size_t len, char * data, int fd)
{
	size_t s;
	while(len > 0 && (s= read(fd, data, len)) > 0){
		data +=s;
		len -=s;
	}
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
        /*
		for(int i = 0; i < 10; i++){
                if(m_list[i] != 0x0 && m_list[i] -> next != 0x0){
                        printf("edge %p -> %p \n", m_list[i] -> mutex , m_list[i] -> next -> mutex);
                }
        }
		*/

		//for(int i = n_history
	for(int i = 0; i < n_edge; i++){
		edge * cur = e_list[i];
		printf("edge %p -> %p (TID : %ld)\n",cur -> start -> mutex, cur -> end -> mutex, cur -> tid);
		printf("mutex history : ");
		for(int j = 0; j < cur -> n_history; j++){
			printf("%p ",cur -> mutex_history[j]);
		}
		printf("\n");
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
	for(int i = 0; i < 10; i++){
		if(t_list[i] != 0x0 && t_list[i] -> tid == tid){
			return t_list[i];
			
		}
	}

	for(int i = 0; i < 10; i++){
		if(t_list[i] == 0x0){
			thread * new_thread = (thread*) malloc(sizeof(thread));
			new_thread -> tid = tid;
			new_thread -> n_mutex = 0;
			t_list[i] = new_thread;
			new_thread -> mutex_list = (node **) malloc(sizeof(node*));
			new_thread -> mutex_list[new_thread -> n_mutex] = 0x0;
			
			return new_thread;
		}
	}
}	

node *
mutex_check_and_create(void * mutex)
{
	for(int i = 0; i < 10; i++){
		if(m_list[i] != 0x0 && m_list[i] -> mutex == mutex){
			return m_list[i];
		}
	}

	for(int i = 0; i < 10; i++){
		if(m_list[i] == 0x0){
			node * new_node;
			new_node = (node*) malloc(sizeof(node));
			new_node -> mutex = mutex;
			new_node -> next = 0x0;
			new_node -> visited = 0;
			m_list[i] = new_node;
			return new_node;
		}
	}
}

void
make_edge(thread * cur_thread)
{
	if(cur_thread -> n_mutex > 0){
		int start = cur_thread -> n_mutex - 1;
		int end = cur_thread -> n_mutex;

		cur_thread -> mutex_list[start] -> next = cur_thread -> mutex_list[end];

		//TODO
		//make edge struct and save it.
		//thread -> mutex_list의 0 부터(not from start) end 까지 타고오면서, mutex 정보들을 하나씩 저장해야 한다.
		edge * new_edge = (edge*)malloc(sizeof(edge)); //program 종료시에 free
		new_edge -> tid = cur_thread -> tid;
		new_edge -> start = cur_thread -> mutex_list[start];
		new_edge -> end = cur_thread -> mutex_list[end];
		//new_edge -> addr = addr;

		new_edge -> mutex_history = (void**) malloc(sizeof(void*) * (cur_thread -> n_mutex));
		new_edge -> n_history = cur_thread -> n_mutex;

		for(int i = 0; i < cur_thread->n_mutex; i++){
			new_edge -> mutex_history[i] = cur_thread -> mutex_list[i] -> mutex;
		}

		e_list = (edge**)realloc(e_list, sizeof(edge) * (n_edge+1) );
		e_list[n_edge] = new_edge;
		n_edge += 1;
		
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
			/*
			if(i - 1 >= 0 && cur_thread -> mutex_list[i - 1] != 0x0){
				cur_thread->mutex_list[i - 1] -> next = cur_thread -> mutex_list[i] -> next;
			}
			*/
				
			cur_thread -> mutex_list[i] = 0x0;
			//cur_node -> next = 0x0;

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
find_line(long addr)
{
	char command[50];
	snprintf(command, 50, "addr2line -e %s %lx",target_path,addr);
	
	FILE * fp = NULL;
	if((fp = popen(command,"r"))==NULL){
		perror("Popen error :");
	}
	
	char buf[512];
	char * data = buf;
	int len = 0;
	for(int s; s = fread(data, 1, sizeof(char), fp); ){
		len += s;
		data += s;
	}

	char * line = strndup(buf,len);
	cnt_line += 1;
	line_info = (char**)realloc(line_info,cnt_line * sizeof(char*));
	line_info[cnt_line - 1] = 0x0;

	for(int i = 0; i < cnt_line; i++){
		if(line_info[i] != 0x0 && strcmp(line_info[i],line)){
			continue;
		}
		line_info[i] = line;
	}
	pclose(fp);	
}

int
find_cycle(node * cur)
{
	//TODO
	// 1.같은 tid edge 로 이뤄진 사이클 거르기. -> 사이클을 찾는 동안 엣지정보를 저장하고,그걸 가지고 판별....
	// 2.gate lock 거르기. 
	int found = 0;
	int count = 0;

	for(node * itr = cur; itr != 0x0; itr = itr -> next){
		if(itr -> visited == 1){
			found = 1;
			count += 1;
			break;
		}

		else{
			itr -> visited = 1;
		}
	}

	for(node * itr = cur; itr != 0x0; itr = itr -> next){
		if(itr -> visited == 1){
			itr -> visited = 0;
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
			find_line(addr);

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
		print("LOCK",tid,mutex,addr);
	
		check_deadlock(addr);
	}

	//UNLOCK
	if(type == 0){
		release(tid,mutex);
				
		//DEBUG
		print("UNLOCK",tid,mutex,addr);
	}			
}

void
handler(int sig)
{
	if(sig == SIGINT){
		if(cnt_line == 0){
			printf("\nNo DeadLock predicted\n");
			exit(0);
		}
		printf("\n-----DeadLock Prediction Result-----\n");
		for(int i =0; i< cnt_line; i++){
			printf("%s\n",line_info[i]);
		}
		exit(0);
	}
}

void
get_arg(int argc, char ** argv)
{
	if(argc != 2){
		printf("Error : Input must be one program path\n");
		exit(1);
	}

	struct stat st;
	if(stat(argv[1],&st) == -1){
		perror("Error : ");
		exit(1);
	}

	target_path = argv[1];	
}

int 
main(int argc, char ** argv)
{
	get_arg(argc,argv);

	signal(SIGINT, handler);

	if(mkfifo(".ddtrace",0666)){
		if(errno != EEXIST){
			perror("fail to open fifo: ");
			exit(1);
		}
	}

	int fd = open(".ddtrace", O_RDWR | O_SYNC);
	if(fd == -1){
		perror("Error :\n");
		exit(1);
	}	

	while (1){
		int type = -1;
		void * mutex = 0x0;
		long tid = -1;
		long addr = 0;		
		read_s(sizeof(type), (char*)&type, fd);
		read_s(sizeof(tid), (char*)&tid, fd);
		read_s(sizeof(mutex), (char*)&mutex, fd);	
		read_s(sizeof(addr), (char*)&addr, fd);
		
		update(type,tid,mutex,addr);		
	}
}


