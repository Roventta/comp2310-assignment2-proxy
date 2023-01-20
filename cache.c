#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"

#include <pthread.h>

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 100000

struct oneCache {
	char* webURL;
	char* webContent;
	int contentSize;
	struct oneCache* nextObject;
	struct oneCache* previousObject;
	int age;
	int access_time;	
};

struct linked_cache_metaData {
	int currentSize;
	struct oneCache* firstObject;	

	struct oneCache* LFU;
	struct oneCache* LRU;
};

/*pthread_rwlock_t rwlock;

struct linked_cache_metaData* LCM;

struct oneCache* cacheStart;
struct oneCache* cacheEnd;
*/

struct oneCache* oneCache_init(char* url, char* content){
	struct oneCache* c = (struct oneCache*) malloc(sizeof(struct oneCache));
	char* c_url = (char*) malloc(sizeof(strlen(url)));
	strcpy(c_url, url);
	char* c_content = (char*) malloc(sizeof(strlen(content)));
	strcpy(c_content, content);

	c->webURL = c_url;
	c->webContent = c_content;

	return c;

}

struct linked_cache_metaData* cacheInit() {
	cacheStart = oneCache_init("start", "start");
	cacheEnd = oneCache_init("end", "end");

	cacheStart->nextObject = cacheEnd;
	cacheEnd->previousObject = cacheStart;


	LCM = (struct linked_cache_metaData*)malloc(sizeof(struct linked_cache_metaData));
	LCM->currentSize = 0;
	LCM->firstObject = cacheStart;
	LCM->firstObject->nextObject = cacheEnd;
	
	pthread_rwlock_init(&rwlock, NULL);

	return LCM;
}

void cacheInsert(char* url, char* content, int a){
	//check whether need to evict 
	while(LCM->currentSize + 2*strlen(content) > MAX_CACHE_SIZE){	
		printf("doing evict in insert\n");
		evict(a);
	}
	if(LCM->currentSize + 2*strlen(content) < MAX_CACHE_SIZE){
	struct oneCache* c = (struct oneCache*) malloc(sizeof(struct oneCache));
	char* c_url = (char*) malloc(sizeof(char)*(strlen(url)+1));
	strcpy(c_url, url);
	char* c_content = (char*) malloc(sizeof(char)*(strlen(content)+1));
	strcpy(c_content, content);
	
	c->webURL = c_url;
	c->webContent = c_content;
	c->contentSize = strlen(content);

	c->age = 0;
	c->access_time = 1;
	//write to metadata, others should not read, wrlock
	pthread_rwlock_wrlock(&rwlock);
	c->nextObject = LCM->firstObject->nextObject;
	LCM->firstObject->nextObject = c;
	LCM->currentSize += c->contentSize;
	
	pthread_rwlock_unlock(&rwlock);
	}
}

char* cacheWalkThrough(char* url){ // return target char if it is in the cache, return "NO" if not;
	char* out = "NO";
	//write to metadata, others should not read, wrlock
	pthread_rwlock_wrlock(&rwlock);
	struct oneCache* c = LCM->firstObject->nextObject;
	
	int oldestAge = 0;
	int smallestFre = 1000;

	while(c!=cacheEnd) {
		c->age ++;
		printf("searching in cache, got %s\n", c->webURL);

		if(strcmp(c->webURL, url) == 0) {
			out = c->webContent;
			c->access_time ++;
			c->age = 0;	
		}
		if(c->age >= oldestAge){
			oldestAge = c->age;
			LCM->LRU = c;
		}
		if(c->access_time <= smallestFre){
			smallestFre = c->access_time;
			LCM->LFU = c;
		}

		c = c->nextObject;	
	}


	pthread_rwlock_unlock(&rwlock);

	return out;
}

void evict(int which){ //input 1 or 0, if 1, evict LRU, if 0, evict LFU
	struct oneCache* c = LCM->firstObject;
	if(which == 1){
		while(c!=cacheEnd){
		if(c==LCM->LRU){
			LCM->currentSize -= c->contentSize;
			c->previousObject->nextObject = c->nextObject;
			c->nextObject->previousObject = c->previousObject;
			free(c->webURL);
			free(c->webContent);
			free(c);	
		}		
		}
	}
	if(which == 0){
		while(c!=cacheEnd){
		if(c==LCM->LFU){
			LCM->currentSize -= c->contentSize;
			c->previousObject->nextObject = c->nextObject;
			c->nextObject->previousObject = c->previousObject;
			free(c->webURL);
			free(c->webContent);
			free(c);	
		}		
		}
	}

	cacheWalkThrough("no");


}
