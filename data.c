#include "data.h"
#include "csapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

BLOB *blob_create(char *content, size_t size){
	BLOB *blob = malloc(sizeof(BLOB));
	if(blob == NULL) return NULL;

	blob->content = strdup(content);//allocate the size of the content
	
	if(blob->content == NULL) return NULL;
	
	blob->size = size;
	blob->refcnt = 1; //reference count initially is 1

	int initRes = pthread_mutex_init(&blob->mutex, NULL); //initialize mutex
	if(initRes != 0){ //error
		free(blob->content);
		free(blob);
		return NULL;
	}

	return blob;

}

BLOB *blob_ref(BLOB *bp, char *why){
	pthread_mutex_lock(&bp->mutex);
	bp->refcnt++;
	pthread_mutex_unlock(&bp->mutex);

	return bp;
}

void blob_unref(BLOB *bp, char *why){
	pthread_mutex_lock(&bp->mutex);
	bp->refcnt--;
	if(bp->refcnt == 0){
		pthread_mutex_unlock(&bp->mutex);
		free(bp->content);
		free(bp);
	} else{
		pthread_mutex_unlock(&bp->mutex);
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
	if(key == NULL) return NULL;

	key->blob = blob_ref(bp, "Key created from blob and inherits caller ref");
	key->hash = blob_hash(bp);
	return key;
}

void key_dispose(KEY *kp){
	blob_unref(kp->blob, "Disposing a key");
	free(kp);

}

int key_compare(KEY *kp1, KEY *kp2){
	if(kp1->hash != kp2->hash) return -1;

	int blobRes = blob_compare(kp1->blob, kp2->blob);

	if(blobRes == -1) return -1;

	return 0;
}

VERSION *version_create(TRANSACTION *tp, BLOB *bp){
	VERSION *version = malloc(sizeof(VERSION));
	if(version == NULL) return NULL;

	version->creator = tp;
	version->blob = blob_ref(bp, "Version created from this blob");
	version->next = NULL;
	version->prev = NULL;

	trans_ref(tp, "Increase ref count of transaction after creating Version");

	return version;
}

void version_dispose(VERSION *vp){
	trans_unref(vp->creator, "Disposing a version");
	blob_unref(vp->blob, "Disposing a version");
	free(vp);

}

//fuser (port #)/tcp
//kill -SIGHUP 4761 
