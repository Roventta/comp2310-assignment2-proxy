#include <stdio.h>
#include <sys/mman.h>
#include "csapp.h"
#include "cache.h"

#include "sbuf.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define SBUFSIZE 10
#define NTHREADS 4

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36\r\n";
static const char* httpMethod = "GET /";
static const char* httpVersion = " HTTP/1.0";
static const char* httpHostname = "\r\nHost: ";
static const char* clientMethod = "GET";
static const char* clientVersion = " HTTP/1.1\n";



sbuf_t sbuf;

int whichEviction;

struct host_and_addr{
	char* host;
	char* addr;
}HAA;

struct host_and_addr badRequest = {"bad request", "bad request"};

char* constructRequest(char * uri, char * hostname){

	char* httpRequest = malloc(MAXLINE);
	
	strcat(httpRequest, httpMethod);
	strcat(httpRequest, uri);
	strcat(httpRequest, httpVersion);
	strcat(httpRequest, httpHostname);
	strcat(httpRequest, hostname);
	strcat(httpRequest, "\r\n\n");
	printf("%s",httpRequest);

	return httpRequest;
}

struct host_and_addr convert_client_query(char* client_query){
	//remove "GET" " HTTP/1.1\n"
	char* http_checker = malloc(7);
	char* https_checker = malloc(8);
	
	int l = strlen(client_query)-strlen(clientMethod)-strlen(clientVersion);
	char* hostAndAddr = (char*)malloc(l);
	strncpy(hostAndAddr, &client_query[strlen(clientMethod)], l);
	printf("combination of host and address is%s\n", hostAndAddr);
	int httpIndex = 0;
	int cmp;
	while(1) {
		strncpy(http_checker, &hostAndAddr[httpIndex], 7);
		strncpy(https_checker, &hostAndAddr[httpIndex], 7);
		printf("%s, %s\n", http_checker, https_checker);
		if((cmp=strcmp(http_checker, "http://")) == 0) {printf("'http://' is located at index: %d\n", httpIndex); httpIndex = httpIndex+7; break;}	
		if((cmp=strcmp(https_checker, "https://")) == 0) {printf("'https://' is located at index: %d\n", httpIndex); httpIndex = httpIndex+8; break;}	
		if(httpIndex > l) {printf("query format error\n"); return badRequest;}
		strncpy(https_checker, &hostAndAddr[httpIndex], 8);
		httpIndex++;
	}

	free(http_checker);
	free(https_checker);

	int i = 0;
	while(1){
		if(hostAndAddr[httpIndex+i] == '/') {printf("'/'located at %d\n", i); break;}	
		if(i>l){printf("query format error\n"); return badRequest;};
		i++;
	}

	char* host = malloc(i);
	char* addr = malloc(l-i);
	strncpy(host, &hostAndAddr[httpIndex], i);
	strncpy(addr, &hostAndAddr[i+httpIndex], l-i-httpIndex);
	printf("host: <%s>\naddr: <%s>\n", host, addr);
	free(hostAndAddr);

	struct host_and_addr out = {host, addr};
	return out;
}

//it also forward content now
void fetchContent(char * hostname, char * port, char * uri, int clientfd) {
	// check weather it is in cache
	char* url = malloc(strlen(uri)+strlen(hostname));
	url = strcat(url, hostname);
	url = strcat(url, uri);

	char* content = cacheWalkThrough(url);
	printf("%s//////////////////\n", content);
	printf("compare result:%d\n", strcmp(content, "NO"));
	if(strcmp(content, "NO") != 0){
	Rio_writen(clientfd, content, strlen(content));
	printf("sent from cache\n");
	return;
	}


	char webLineBuf[MAX_OBJECT_SIZE];
	char lastEight[8];

	rio_t rio;
	memset(lastEight, 0, strlen(lastEight));

	int serverfd = open_clientfd(hostname, port);
	if(serverfd<0){
		printf("open clientfd error");
		return;
	}
	printf("proxy to server opened\n");
	char* httpRequest = constructRequest(uri, hostname);
	Rio_readinitb(&rio, serverfd);
	printf("reader inited\n");

	Rio_writen(serverfd, httpRequest, strlen(httpRequest));
	printf("request sent\n");
	/*int result;
	int i = 0;
	while(result = strcmp(lastEight, "</html>\n") != 0){
		lseek(clientfd, 20*i, SEEK_SET); 
		rio_readnb(&rio, webLineBuf, 1);
		strcpy(lastEight, &webLineBuf[strlen(webLineBuf)-8]);
		printf("%s\n",lastEight);
		printf("compare result %d\n", result);
		//printf("%s",webContentBuf);
		Rio_writen(clientfd, webLineBuf, MAXLINE);
		printf("sent\n");
		i++;
	}
*/

	rio_readnb(&rio, webLineBuf, MAX_OBJECT_SIZE);
	Rio_writen(clientfd, webLineBuf, strlen(webLineBuf));

	printf("sent\n");

	cacheInsert(url, webLineBuf, whichEviction);
	printf("cache inserted\n");

	free(hostname);
	free(uri);

	free(httpRequest);

	Close(serverfd);
}

void handle_client_query(int connfd){
	char buf[MAXLINE];
	rio_t rio;

	Rio_readinitb(&rio, connfd);

	//read one line of request
	Rio_readlineb(&rio, buf, MAXLINE);
	printf("got request that is: %s\n", buf);
	struct host_and_addr HAA = convert_client_query(buf);
	if(strcmp(HAA.host, badRequest.host) == 0) {printf("client sent bad request\n");return;}
	fetchContent(HAA.host, "80", HAA.addr, connfd);
	
	Close(connfd);
}

void* thread(void *vargp) {
	printf("got thread\n");
	Pthread_detach(pthread_self());	
	while(1) {
		int connfd = sbuf_remove(&sbuf);
		//process connection;
		printf("thread processing connection fd <%d>\n", connfd);
		handle_client_query(connfd); // connection closed at here	
	}	
}

int main(int argc, char **argv)
{
	/*
//	printf("%s", user_agent_hdr);
    
//	fetchContent("www.example.com\0", "80\0", "\0");
//	struct host_and_addr data = convert_client_query("GET http://www.example.com/index.html HTTP/1.1");
//	fetchContent(data.host, "80", data.addr);

	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	char client_hostname[MAXLINE], client_port[MAXLINE];

	listenfd = Open_listenfd(argv[1]);

	while(1) {
		clientlen = sizeof(struct sockaddr_storage);
		connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
		printf("Conected to (%s, %s)\n", client_hostname, client_port);

		handle_client_query(connfd);

	}
	exit(0);
	*/
	int  listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;
	pthread_t tid;

	listenfd = open_listenfd(argv[1]);
	whichEviction = atoi(argv[2]);

	sbuf_init(&sbuf,4);

	cacheInit();

	for(int i=0; i<NTHREADS; i++) {
		Pthread_create(&tid, NULL, thread, NULL);	
	}
	while(1) {
		clientlen = sizeof(struct sockaddr_storage);
		connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);
		sbuf_insert(&sbuf, connfd);
	}

	return 666;
}
