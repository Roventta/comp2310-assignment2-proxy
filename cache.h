#ifndef COMP2310_CACHE_H
#define COMP2310_CACHE_H

#include <stdlib.h>
#include <pthread.h>
#include "csapp.h"

struct oneCache;
struct linked_cache_metaData;

struct oneCache* cacheStart;
struct oneCache* cacheEnd;
struct linked_cache_metaData* LCM;
pthread_rwlock_t rwlock;

struct linked_cache_metaData* cacheInit();
void cacheInsert (char* url, char* content, int which);
char* cacheWalkThrough(char* url);
void evict(int which);


#endif

