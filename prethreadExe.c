#include "sbuf.h"

#include <pthread.h>

#define NTHREADS 4

sbuf_t sbuf;

void* thread(void *vargp) {
	printf("a\n");
	Pthread_detach(pthread_self());
	while(1) {
		int threadTask = sbuf_remove(&sbuf);
		printf("thread processing task with number <%d>\n", threadTask);

	}
}


int main(){
	int i, listenfd, connfd;
	pthread_t tid;
	char inputBuf[MAXLINE];
	sbuf_init(&sbuf, 4);

	for(int i=0; i<NTHREADS; i++) {
		Pthread_create(&tid, NULL, thread, NULL);
	}	
	while(1){
		while(Fgets(inputBuf, MAXLINE, stdin) != NULL){
			int threadTask = atoi(inputBuf);
			printf("main thread receive request: <%d>\n", threadTask);
			//char* threadTask = malloc(MAXLINE);
			//strcpy(threadTask, inputBuf);
			sbuf_insert(&sbuf, threadTask); // insert task into buffer
		}
	}
}
