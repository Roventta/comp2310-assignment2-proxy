#include <stdio.h>

#include "cache.c"

void walk(){
	struct oneCache* c = cacheStart;
	while(c!=cacheEnd){
	printf("%s\n", c->webURL);
	printf("%s\n", c->webContent);
	printf("age: %d\n", c->age);
	printf("access number: %d\n", c->access_time);
	c = c->nextObject;
	}

	printf("///walk end///\n");
}

void illustrateMeta(){
	printf("LRU is %s\n",LCM-> LRU->webURL);
	printf("LFU is %s\n",LCM-> LFU->webURL);
}

int main(){
	cacheInit();
	cacheInsert("first", "first", 1);
	cacheInsert("second", "second", 1);

	walk();
	
	printf("searching for first, %s\n",cacheWalkThrough("second"));
	printf("searching for first, %s\n",cacheWalkThrough("second"));
	printf("searching for first, %s\n",cacheWalkThrough("second"));
	printf("searching for first, %s\n",cacheWalkThrough("seco"));

	illustrateMeta();

	walk();
	
	printf("searching for third, %s\n",cacheWalkThrough("third"));

	illustrateMeta();
	//evict("");

}
