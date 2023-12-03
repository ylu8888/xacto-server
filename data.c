#include "data.h"
#include "csapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BLOB *blob_create(char *content, size_t size){
	BLOB *blob = malloc(sizeof(BLOB));
	if(blob == NULL) return NULL;

	blob->content = malloc(sizeof(size)); //allocate the size of the content
	if(blob->content == NULL) return NULL;
	memcpy(blob->content, content, size);
	blob->size = size;
	blob->refcnt = 1; //reference count initially is 1

	//Sem_init(&blob->mutex, 0, 1); //blob->mutex is of type pthread_mutex but sem_t expects type sem_t

	return blob;

}

BLOB *blob_ref(BLOB *bp, char *why){
	bp->refcnt++;

	return bp;
}

void blob_unref(BLOB *bp, char *why){
	bp->refcnt--;
	if(bp->refcnt == 0){
		free(bp->content);
		free(bp);
	}

}

int blob_compare(BLOB *bp1, BLOB *bp2){
	if(strcmp(bp1->content, bp2->content) == 0){
		return 0;
	}
	
	return -1;
}

int blob_hash(BLOB *bp){
	int hashBrown = 7919; 
	char* contentFrog = bp->content;

	while(*contentFrog){
		hashBrown = ((hashBrown << 5) + hashBrown) + *contentFrog;
		contentFrog++;

	}

	return hashBrown;
}

KEY *key_create(BLOB *bp){
	KEY *key = malloc(sizeof(KEY));
	return key;
}

void key_dispose(KEY *kp){

}

int key_compare(KEY *kp1, KEY *kp2){

	return 0;
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){
	VERSION *version = malloc(sizeof(VERSION));
	return version;
}

void version_dispose(VERSION *vp){

}
