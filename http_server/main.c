/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 * 
 * 
 * Marvelously Modified by:
 * 
 * Liam Cusack
 * lrcusack@gwmail.gwu.edu
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "link.h"
#include "ring.h"
//#include "chardev.h"

#include <util.h> 		/* client_process */
#include <server.h>		/* server_accept and server_create */

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 256
#define INTBUF sizeof(int)

pthread_mutex_t* thor;
pthread_cond_t* ironman;
struct ll* requests_l;
struct ring* requests_r;
static int qdev;


/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests_l.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void
server_single_request(int accept_fd)
{
	int fd;

	/* 
	 * The server thread will always want to be doing the accept.
	 * That main thread will want to hand off the new fd to the
	 * new threads/processes/thread pool.
	 */
	fd = server_accept(accept_fd);
	client_process(fd);

	return;
}

void
server_multiple_requests(int accept_fd)
{
	int fd;

	while(1){
		fd = server_accept(accept_fd);
		client_process(fd);
	}
	return;
}

void
server_processes(int accept_fd)
{
	
	int fd;
	pid_t pid;
	int count = 0;
	while(1){
		if(count  <= MAX_CONCURRENCY){
			
			fd = server_accept(accept_fd);
			pid = fork();
			if (pid > 0) {
				count++;
				printf("child created, count is %d\n",count);
			}
			if (pid < 0) printf("I told the judge, I said 'that aint my child'\n");
			if (pid == 0){
				client_process(fd);
				exit((int) NULL);
			}
		}
		if(waitpid(-1,(int*) NULL,WNOHANG) > 0) {
			count--;
			printf("child ended, count is %d\n",count);
		}
	}
	
	return;
}

void
server_dynamic(int accept_fd)
{
	accept_fd++;
	return;
}

void
server_thread_per(int accept_fd)
{
	accept_fd++;
	return;
}

void
server_task_queue(int accept_fd)
{
	accept_fd++;
	return;
}


void*
mutex_worker()
{
//worker routine
	int fd;
	//int* fdp;

	while(1){
		//printf("getting lock\n");
		pthread_mutex_lock(thor);
		
		//printf("dequeuing from requests_l\n");
		fd =(int) ll_dequeue(requests_l);
		//printf("releasing lock\n");
		pthread_mutex_unlock(thor);
		if(fd!=-1){
			//printf("holla, we got one, its %d\n",fd);
			client_process(fd);
			//printf("handled client process %d\n",fd);
			//free(&fd);
		}
	}
}

void*
conditional_worker()
{
	//worker routine
	int fd;
	//int* fdp;

	while(1){
		//printf("getting lock\n");
		pthread_mutex_lock(thor);
		while(rb_isempty(requests_r)) pthread_cond_wait(ironman,thor);
		//printf("dequeuing from requests_r\n");
		fd =(int) rb_dequeue(requests_r);
		//printf("sending signal\n");
		pthread_cond_signal(ironman);
		//printf("releasing lock\n");
		pthread_mutex_unlock(thor);
		if(fd!=-1){
			//printf("holla, we got one, its %d\n",fd);
			client_process(fd);
			//printf("handled client process %d\n",fd);
		}
	}
	return NULL;
}

void
server_thread_pool(int accept_fd)
{
	thor = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	requests_l = ll_create();	
	pthread_mutex_init(thor,NULL);
	
	//initiate all threads
	pthread_t pool[MAX_CONCURRENCY];
	int ii;
	for(ii=0; ii<MAX_CONCURRENCY; ii++){
		pthread_create(&pool[ii],NULL,&mutex_worker,NULL);
		//printf("made thread: %d\n",(int) pool[ii]);
	}
	//end initiate threads
	
	//server process


	int fd;

	while(1){
		fd = server_accept(accept_fd);
		
		pthread_mutex_lock(thor);
		//printf("adding %d to runqueue\n",fd);
		ll_enqueue(requests_l, (void*) fd);
		pthread_mutex_unlock(thor);
	}
	//end server process
		

	
	return;
}
void
server_thread_pool_blocking(int accept_fd)
{
	//initialize stuff
	ironman = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
	thor = (pthread_mutex_t*) malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(thor,NULL);
	pthread_cond_init(ironman,NULL);
	requests_r = rb_create();
	pthread_t pool[MAX_CONCURRENCY];
	int ii;
	int fd;

	//-----------------------------------------------
	//create threads
	for(ii=0; ii<MAX_CONCURRENCY; ii++){
		pthread_create(&pool[ii],NULL,&conditional_worker,NULL);
		//printf("made thread: %d\n",(int) pool[ii]);
	}
	//-------------------------------------------------
	//run master process
	while(1){
		fd = server_accept(accept_fd);
		//printf("got fd: %d\n",fd);
		//printf("locking mutex in main\n");
		pthread_mutex_lock(thor);
		//printf("checking conditional\n");
		if(rb_isfull(requests_r)) {
			//printf("waiting...\n");
			pthread_cond_wait(ironman,thor);
		}
		//printf("adding %d to runqueue\n",fd);
		rb_enqueue(requests_r, (void*) fd);
		//printf("unlocking mutex in main \n");
		pthread_cond_signal(ironman);
		pthread_mutex_unlock(thor);
	}

	
	return;
}

void*
kernel_worker()
{
//worker routine
	//int qdev = (int) qdev_p;
	char fd_b[INTBUF];
	int fd;

	while(1){
		if(read(qdev,fd_b,INTBUF)){
			fd = (int) fd_b;
			client_process(fd);
		}
	}
}

void
server_char_device_queue(int accept_fd)
{
	printf("called server subroutine\n");
	qdev = open("/dev/osqueue",O_RDWR);
	printf("got file descriptor for osqueue\n");
	int fd;
	
	pthread_t pool[MAX_CONCURRENCY];
	int ii;

	//create threads
	for(ii=0; ii<MAX_CONCURRENCY; ii++){
		pthread_create(&pool[ii],NULL,&kernel_worker,NULL);
		//printf("made thread: %d\n",(int) pool[ii]);
	}
	printf("created threads\n");
	while(1){
		fd = server_accept(accept_fd);
		printf("accepted request\n");
		write(qdev, (const char*) fd, INTBUF);	
		printf("wrote to device\n");	
	}
	
	close(qdev);
	return;
}

void
server_wait_queue(int accept_fd)
{
	accept_fd++;
	return;
}


typedef enum {
	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_SINGLET,
	SERVER_TYPE_PROCESS,
	SERVER_TYPE_FORK_EXEC,
	SERVER_TYPE_SPAWN_THREAD,
	SERVER_TYPE_TASK_QUEUE,
	SERVER_TYPE_THREAD_POOL,
	SERVER_TYPE_THREAD_POOL_BLOCKING,
	SERVER_TYPE_CHAR_DEVICE_QUEUE,
	SERVER_TYPE_WAIT_QUEUE,
} server_type_t;

int
main(int argc, char *argv[])
{
	server_type_t server_type;
	short int port;
	int accept_fd;

	if (argc != 3) {
		printf("Proper usage of http server is:\n%s <port> <#>\n"
		       "port is the port to serve on, # is either\n"
		       "0: serve only a single request\n"
		       "1: use only a single thread for multiple requests_l\n"
		       "2: use fork to create a process for each request\n"
		       "3: Extra Credit: use fork and exec when the path is an executable to run the program dynamically.  This is how web servers handle dynamic (program generated) content.\n"
		       "4: create a thread for each request\n"
		       "5: use atomic instructions to implement a task queue\n"
		       "6: use a thread pool\n"
		       "7: to be defined\n"
		       "8: to be defined\n"
		       "9: to be defined\n",
		       argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	accept_fd = server_create(port);
	if (accept_fd < 0) return -1;
	
	server_type = atoi(argv[2]);

	switch(server_type) {
	case SERVER_TYPE_ONE:
		server_single_request(accept_fd);
		break;
	case SERVER_TYPE_SINGLET:
		server_multiple_requests(accept_fd);
		break;
	case SERVER_TYPE_PROCESS:
		server_processes(accept_fd);
		break;
	case SERVER_TYPE_FORK_EXEC:
		server_dynamic(accept_fd);
		break;
	case SERVER_TYPE_SPAWN_THREAD:
		server_thread_per(accept_fd);
		break;
	case SERVER_TYPE_TASK_QUEUE:
		server_task_queue(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL:
		server_thread_pool(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_BLOCKING:
		server_thread_pool_blocking(accept_fd);
		break;
	case SERVER_TYPE_CHAR_DEVICE_QUEUE:
		server_char_device_queue(accept_fd);
		break;
	case SERVER_TYPE_WAIT_QUEUE:
		server_wait_queue(accept_fd);
		break;
	
	}
	close(accept_fd);

	return 0;
}
